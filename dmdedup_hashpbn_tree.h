#ifndef DMDEDUP_HASHPBN_CHECKER_H
#define DMDEDUP_HASHPBN_CHECKER_H

#include "persistent-data/data-structures/btree.h"
#include "persistent-data/run.h"

//----------------------------------------------------------------

namespace dmdedup {
	namespace hashpbn_tree_detail {

                struct hash_pbn_value {
                        uint64_t pbn;  
                };
              
		struct hashpbn_details_traits {
                        typedef base::le64 disk_type;
                        typedef hash_pbn_value value_type;
			typedef persistent_data::no_op_ref_counter<hash_pbn_value> ref_counter;
                        static void unpack(disk_type const &disk, value_type &value);
			static void pack(value_type const &value, disk_type &disk);
		};

		class damage_visitor;

		struct damage {
			virtual ~damage() {}
			virtual void visit(damage_visitor &v) const = 0;
		};

		struct missing_hashes : public damage {
			missing_hashes(std::string const &desc, run<uint64_t> const &keys);
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

			virtual void visit(missing_hashes const &d) = 0;
		};

		// FIXME: need to add some more damage types for bad leaf data

		class device_visitor {
		public:
			virtual ~device_visitor() {}
			virtual void visit(block_address hash, hash_pbn_value const &v) = 0;
		};
	};

	typedef persistent_data::btree<1, hashpbn_tree_detail::hashpbn_details_traits> hashpbn_tree;

	void walk_hashpbn_tree(hashpbn_tree const &tree,
			      hashpbn_tree_detail::device_visitor &dev_v,
			      hashpbn_tree_detail::damage_visitor &dv);
	void check_hashpbn_tree(hashpbn_tree const &tree,
			       hashpbn_tree_detail::damage_visitor &visitor);
}

//----------------------------------------------------------------

#endif
