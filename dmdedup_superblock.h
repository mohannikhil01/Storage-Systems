#ifndef DMDEDUP_SUPERBLOCK_H
#define DMDEDUP_SUPERBLOCK_H

#include "base/endian_utils.h"

#include "persistent-data/block.h"
#include "persistent-data/data-structures/ref_counter.h"

//----------------------------------------------------------------

namespace dmdedup {
	namespace superblock_detail {
		using namespace base;
		using namespace persistent_data;

		unsigned const SPACE_MAP_ROOT_SIZE = 128;

		typedef unsigned char __u8;

		struct superblock_disk {
			le32 csum_;
			le32 flags_;
			le64 blocknr_;

			__u8 uuid_[16];
			le64 magic_;
			le32 version_;

			__u8 data_space_map_root_[SPACE_MAP_ROOT_SIZE];
			__u8 metadata_space_map_root_[SPACE_MAP_ROOT_SIZE];

			/* btree mapping lbn -> pbn */
			le64 lbn_pbn_root_;

			/* btree mapping hash -> pbn */
			le64 hash_pbn_root_;

			le32 data_block_size_; /* in 512-byte sectors */

			le32 metadata_block_size_; /* in 512-byte sectors */
			le64 metadata_nr_blocks_;

		} __attribute__ ((packed));

		struct superblock {
			uint32_t csum_;
			uint32_t flags_;
			uint64_t blocknr_;

			unsigned char uuid_[16];
			uint64_t magic_;
			uint32_t version_;

			unsigned char data_space_map_root_[SPACE_MAP_ROOT_SIZE];
			unsigned char metadata_space_map_root_[SPACE_MAP_ROOT_SIZE];

			/* btree mapping lbn -> pbn */
			uint64_t lbn_pbn_root_;

			/* btree mapping hash -> pbn */
			uint64_t hash_pbn_root_;

			uint32_t data_block_size_; /* in 512-byte sectors */

			uint32_t metadata_block_size_; /* in 512-byte sectors */
			uint64_t metadata_nr_blocks_;

			bool get_needs_check_flag() const;
			void set_needs_check_flag(bool val = true);
		};

		struct superblock_traits {
			typedef superblock_disk disk_type;
			typedef superblock value_type;
			typedef no_op_ref_counter<superblock> ref_counter;

			static void unpack(superblock_disk const &disk, superblock &core);
			static void pack(superblock const &core, superblock_disk &disk);
		};

		block_address const SUPERBLOCK_LOCATION = 0;
//                #define SUPERBLOCK_MAGIC 2126509509
		uint32_t const SUPERBLOCK_MAGIC = 2126509509;

		//--------------------------------

		class damage_visitor;

		struct damage {
			virtual ~damage() {}
			virtual void visit(damage_visitor &v) const = 0;
		};

		struct superblock_corruption : public damage {
			superblock_corruption(std::string const &desc);
			void visit(damage_visitor &v) const;

			std::string desc_;
		};

		class damage_visitor {
		public:
			virtual ~damage_visitor() {}

			void visit(damage const &d);

			virtual void visit(superblock_corruption const &d) = 0;
		};
	}

	bcache::validator::ptr superblock_validator();

	// FIXME: should we put init_superblock in here too?

	superblock_detail::superblock read_superblock(persistent_data::block_manager<>::ptr bm);
	superblock_detail::superblock read_superblock(persistent_data::block_manager<>::ptr bm,
						      persistent_data::block_address location);

	void write_superblock(persistent_data::block_manager<>::ptr bm,
			      superblock_detail::superblock const &sb);

	void check_superblock(persistent_data::block_manager<>::ptr bm,
			      superblock_detail::damage_visitor &visitor);
}

//----------------------------------------------------------------

#endif
