/*

    File: file_bdb.c

    Copyright (C) 2008 Christophe GRENIER <grenier@cgsecurity.org>
  
    This software is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
  
    You should have received a copy of the GNU General Public License along
    with this program; if not, write the Free Software Foundation, Inc., 51
    Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 */

#if !defined(SINGLE_FORMAT) || defined(SINGLE_FORMAT_berkeleydb)
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <stdio.h>
#include "types.h"
#include "filegen.h"
#include "common.h"

/*@ requires valid_register_header_check(file_stat); */
static void register_header_check_berkeleydb(file_stat_t *file_stat);

const file_hint_t file_hint_berkeleydb= {
  .extension="dat",
  .description="berkeleydb database file",
  .max_filesize=PHOTOREC_MAX_FILE_SIZE,
  .recover=1,
  .enable_by_default=1,
	.register_header_check=&register_header_check_berkeleydb
};

struct __db_lsn { /* SHARED */
	uint32_t	file;		/* File ID. */
	uint32_t	offset;		/* File offset. */
};

struct db_header {
	uint32_t	  lsn;		/* 00-07: LSN. */
	uint32_t pgno;		/* 08-11: Current page number. */
	uint32_t magic;	/* 12-15: Magic number. */
	uint32_t version;	/* 16-19: Version. */
	uint32_t pagesize;	/* 20-23: Pagesize. */
	uint8_t  encrypt_alg;	/*    24: Encryption algorithm. */
	uint8_t  type;		/*    25: Page type. */
	uint8_t  metaflags;	/* 26: Meta-only flags */
	uint8_t  unused1;	/* 27: Unused. */
	uint32_t free;		/* 28-31: Free list page number. */
	uint32_t last_pgno;	/* 32-35: Page number of last page in db. */
	uint32_t nparts;	/* 36-39: Number of partitions. */
	uint32_t key_count;	/* 40-43: Cached key count. */
	uint32_t record_count;	/* 44-47: Cached record count. */
	uint32_t flags;	/* 48-51: Flags: unique to each AM. */
				/* 52-71: Unique file ID. */
	uint8_t  uid[20];
} __attribute__ ((gcc_struct, __packed__));
//} __attribute__ ((gcc_struct));

struct old_db_header
{
 char magic[16];
 uint16_t pagesize;
 uint8_t  ffwrite;
 uint8_t  ffread;
 uint8_t  reserved;
 uint8_t  max_emb_payload_frac;
 uint8_t  min_emb_payload_frac;
 uint8_t  leaf_payload_frac;
 uint32_t file_change_counter;
 uint32_t filesize_in_page;
 uint32_t first_freelist_page;
 uint32_t freelist_pages;
 uint32_t schema_cookie;
 uint32_t schema_format;
 uint32_t default_page_cache_size;
 uint32_t largest_root_btree;
 uint32_t text_encoding;
 uint32_t user_version;
 uint32_t inc_vacuum_mode;
 uint32_t app_id;
 char     reserved_for_expansion[20];
 uint32_t version_valid_for;
 uint32_t version;
} __attribute__ ((gcc_struct, __packed__));

/*@
  @ requires buffer_size >= sizeof(struct db_header);
  @ requires separation: \separated(&file_hint_sqlite, buffer+(..), file_recovery, file_recovery_new);
  @ requires valid_header_check_param(buffer, buffer_size, safe_header_only, file_recovery, file_recovery_new);
  @ ensures  valid_header_check_result(\result, file_recovery_new);
  @ assigns  *file_recovery_new;
  @*/
static int header_check_berkeleydb(const unsigned char *buffer, const unsigned int buffer_size, const unsigned int safe_header_only, const file_recovery_t *file_recovery, file_recovery_t *file_recovery_new)
{
  const struct db_header *hdr=(const struct db_header *)buffer;
  unsigned int pagesize=be16(hdr->pagesize);
  const unsigned int filesize_in_page=be32(hdr->last_pgno + 1);

  reset_file_recovery(file_recovery_new);
#ifdef DJGPP
  file_recovery_new->extension="db";
#else
  file_recovery_new->extension=file_hint_berkeleydb.extension;
#endif
  file_recovery_new->min_filesize=sizeof(struct db_header);
  if(filesize_in_page!=0)
  {
    file_recovery_new->calculated_file_size=(uint64_t)filesize_in_page * pagesize;
    file_recovery_new->data_check=&data_check_size;
    file_recovery_new->file_check=&file_check_size;
  }
  return 1;
}

static void register_header_check_berkeleydb(file_stat_t *file_stat)
{
  static const unsigned char berkeleydb_header[16]= { 0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0x31,0x05,0x00 };
  register_header_check(0, berkeleydb_header, 16, &header_check_berkeleydb, file_stat);
}
#endif
