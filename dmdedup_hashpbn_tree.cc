#include "dm-dedup/hashpbn_tree.h"

#include "persistent-data/data-structures/btree_damage_visitor.h"

using namespace persistent_data;
using namespace dmdedup;

//----------------------------------------------------------------

namespace {
	using namespace hashpbn_tree_detail;

	struct visitor_adapter {
		visitor_adapter(device_visitor &dv)
		: dv_(dv) {
		}

		void visit(btree_path const &path, hash_pbn_value const &dd) {
			dv_.visit(path[0], dd);
		}

	private:
		device_visitor &dv_;
	};

	// No op for now, should add sanity checks in here however.
	struct noop_visitor : public device_visitor {
		virtual void visit(block_address hash, hash_pbn_value const &dd) {
		}
	};

	class ll_damage_visitor {
	public:
		// FIXME: is the namespace needed on here?
		ll_damage_visitor(hashpbn_tree_detail::damage_visitor &v)
		: v_(v) {
		}

		virtual void visit(btree_path const &path, btree_detail::damage const &d) {
			v_.visit(missing_hashes(d.desc_, d.lost_keys_));
		}

	private:
		hashpbn_tree_detail::damage_visitor &v_;
	};
}

namespace dmdedup {
	namespace hashpbn_tree_detail {
		void
		hashpbn_details_traits::unpack(disk_type const &disk, value_type &value)
		{
                        cout << "Hash to pbn unpack \n";
                        value.pbn = to_cpu<uint64_t>(disk);
		}

		void
		hashpbn_details_traits::pack(value_type const &value, disk_type &disk)
		{
                        disk = to_disk<le64>(value.pbn);
		}

		missing_hashes::missing_hashes(std::string const &desc,
						 run<uint64_t> const &keys)
			: desc_(desc),
			  keys_(keys) {
		}

		void missing_hashes::visit(damage_visitor &v) const {
			v.visit(*this);
		}
	}
}

//----------------------------------------------------------------

void
dmdedup::walk_hashpbn_tree(hashpbn_tree const &tree,
				    hashpbn_tree_detail::device_visitor &vv,
				    hashpbn_tree_detail::damage_visitor &dv)
{
	visitor_adapter av(vv);
	ll_damage_visitor ll_dv(dv);
	btree_visit_values(tree, av, ll_dv);
}

void
dmdedup::check_hashpbn_tree(hashpbn_tree const &tree, damage_visitor &visitor)
{
        cout << "Calling check hashpbn tree \n" ;
	noop_visitor vv;
	walk_hashpbn_tree(tree, vv, visitor);
}

//----------------------------------------------------------------
