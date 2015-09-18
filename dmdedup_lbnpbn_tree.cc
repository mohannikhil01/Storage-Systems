#include "dm-dedup/lbnpbn_tree.h"

#include "persistent-data/data-structures/btree_damage_visitor.h"

using namespace persistent_data;
using namespace dmdedup;

//----------------------------------------------------------------

namespace {
	using namespace lbnpbn_tree_detail;

	struct visitor_adapter {
		visitor_adapter(device_visitor &dv)
		: dv_(dv) {
		}

		void visit(btree_path const &path, lbn_pbn_value const &dd) {
			dv_.visit(path[0], dd);
		}

	private:
		device_visitor &dv_;
	};

	// No op for now, should add sanity checks in here however.
	struct noop_visitor : public device_visitor {
		virtual void visit(block_address lbn, lbn_pbn_value const &dd) {
		}
	};

	class ll_damage_visitor {
	public:
		// FIXME: is the namespace needed on here?
		ll_damage_visitor(lbnpbn_tree_detail::damage_visitor &v)
		: v_(v) {
		}

		virtual void visit(btree_path const &path, btree_detail::damage const &d) {
			v_.visit(missing_lbns(d.desc_, d.lost_keys_));
		}

	private:
		lbnpbn_tree_detail::damage_visitor &v_;
	};
}

namespace dmdedup {
	namespace lbnpbn_tree_detail {
		void
		lbnpbn_details_traits::unpack(disk_type const &disk, value_type &value)
		{
                        cout << "Lbn to pbn unpack \n";
                        value.pbn = to_cpu<uint64_t>(disk);
		}

		void
		lbnpbn_details_traits::pack(value_type const &value, disk_type &disk)
		{
                        disk = to_disk<le64>(value.pbn);
		}

		missing_lbns::missing_lbns(std::string const &desc,
						 run<uint64_t> const &keys)
			: desc_(desc),
			  keys_(keys) {
		}

		void missing_lbns::visit(damage_visitor &v) const {
			v.visit(*this);
		}
	}
}

//----------------------------------------------------------------

void
dmdedup::walk_lbnpbn_tree(lbnpbn_tree const &tree,
				    lbnpbn_tree_detail::device_visitor &vv,
				    lbnpbn_tree_detail::damage_visitor &dv)
{
	visitor_adapter av(vv);
	ll_damage_visitor ll_dv(dv);
	btree_visit_values(tree, av, ll_dv);
}

void
dmdedup::check_lbnpbn_tree(lbnpbn_tree const &tree, damage_visitor &visitor)
{
        cout << "Calling check lbnpbn tree \n" ;
	noop_visitor vv;
	walk_lbnpbn_tree(tree, vv, visitor);
}

//----------------------------------------------------------------
