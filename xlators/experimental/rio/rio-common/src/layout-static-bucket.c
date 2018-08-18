/*
  Copyright (c) 2016 Red Hat, Inc. <http://www.redhat.com>
  This file is part of GlusterFS.

  This file is licensed to you under your choice of the GNU Lesser
  General Public License, version 3 or any later version (LGPLv3 or
  later), or the GNU General Public License, version 2 (GPLv2), in all
  cases as published by the Free Software Foundation.
*/

/* File: layout-static-bucket.c
 * Static bucket layout, assumes the bucket is encoded in the passed in search
 * parameter. When this is a GFID its upper 2 bytes are considered as the
 * bucket index.
 */

#include "rio-mem-types.h"
#include "layout.h"
#include "rio-common.h"

#define MAX_BUCKETS 65536

struct layout_ops layout_static_bucket;

struct layout *
layout_staticbucket_init (char *layouttype, int count,
                          struct rio_subvol *xlators, dict_t *options)
{
        int i;
        struct layout *new_layout = NULL;

        if (!layouttype || !xlators || !options || count <= 0)
                goto out;

        if (strncmp (LAYOUT_STATIC_BUCKET, layouttype,
                     strlen(LAYOUT_STATIC_BUCKET)) != 0) {
                goto out;
        }

        new_layout = GF_CALLOC (1, sizeof (struct layout), gf_rio_mt_layout);
        if (!new_layout)
                goto out;

        new_layout->layou_ops = &layout_static_bucket;

        /* allocate a contiguous (array) of xlator_t pointers */
        new_layout->layou_buckets = GF_CALLOC (MAX_BUCKETS, sizeof (xlator_t *),
                                         gf_rio_mt_layout_static_bucket);
        if (!new_layout->layou_buckets) {
                GF_FREE (new_layout);
                new_layout = NULL;
                goto out;
        }

        for (i = 0; i < MAX_BUCKETS;) {
                struct rio_subvol *subvol = NULL;

                list_for_each_entry (subvol, &xlators->riosvl_node,
                                     riosvl_node) {
                        new_layout->layou_buckets[i] = subvol->riosvl_xlator;
                        i++;
                        if (i >= MAX_BUCKETS)
                                break;
                }
        }
#ifdef RIO_DEBUG
        {
                for (i = 0; i < (MAX_BUCKETS < 16 ? MAX_BUCKETS : 16); i++)
                        gf_msg ("layout", GF_LOG_INFO, 0, 0,
                                "StaticBucket: %d %s",
                                i, new_layout->layou_buckets[i]->name);
        }
#endif
out:
        return new_layout;
}

void
layout_staticbucket_destroy (struct layout *layout)
{
        GF_FREE (layout->layou_buckets);
        GF_FREE (layout);

        return;
}

xlator_t *
layout_staticbucket_search (struct layout *layout, uuid_t gfid)
{
        return layout->layou_buckets[gfid_to_bucket (gfid)];
}

struct layout_ops layout_static_bucket = {
        .laops_init = layout_staticbucket_init,
        .laops_destroy = layout_staticbucket_destroy,
        .laops_search = layout_staticbucket_search
};
