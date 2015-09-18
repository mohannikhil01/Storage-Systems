#ifndef DMDEDUP_LBNPBN_CHECKER_H
#define DMDEDUP_LBNPBN_CHECKER_H

#include "persistent-data/data-structures/btree.h"
#include "persistent-data/run.h"

//----------------------------------------------------------------

namespace dmdedup {
	namespace lbnpbn_tree_detail {

                struct lbn_pbn_value {
                        uint64_t pbn;  
                };
              
		struct lbnpbn_details_traits {
                        typedef base::le64 disk_type;
                        typedef lbn_pbn_value value_type;
			typedef persistent_data::no_op_ref_counter<lbn_pbn_value> ref_counter;
                        static void unpack(disk_type const &disk, value_type &value);
			static void pack(value_type const &value, disk_type &disk);
		};

		class damage_visitor;

		struct damage {
			virtual ~damage() {}
			virtual void visit(damage_visitor &v) const = 0;
		};

		struct missing_lbns : public damage {
			missing_lbns(std::string const &desc, run<uint64_t> const &keys);
			virtual void visit(damage_visitor &v) const;

			std::string desc_;
			run<uint64_t> keys_;
		};

		class damage_visitor {
		public:
			typedef boost::shared_ptr<damage_visitor> ptr;

			virtual ~damage_visitor() {}

			void visit(damage const &d) {
				d.visit(*this);
			}

			virtual void visit(missing_lbns const &d) = 0;
		};

		// FIXME: need to add some more damage types for bad leaf data

		class device_visitor {
		public:
			virtual ~device_visitor() {}
			virtual void visit(block_address lbn, lbn_pbn_value const &v) = 0;
		};
	};

	typedef persistent_data::btree<1, lbnpbn_tree_detail::lbnpbn_details_traits> lbnpbn_tree;

	void walk_lbnpbn_tree(lbnpbn_tree const &tree,
			      lbnpbn_tree_detail::device_visitor &dev_v,
			      lbnpbn_tree_detail::damage_visitor &dv);
	void check_lbnpbn_tree(lbnpbn_tree const &tree,
			       lbnpbn_tree_detail::damage_visitor &visitor);
}

//----------------------------------------------------------------

#endif
