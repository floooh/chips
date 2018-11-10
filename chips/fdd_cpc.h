#pragma once
/*#
    # fdd_cpc.h

    Extension functions for fdd.h to load Amstrad CPC emulator disk
    image formats into fdd_t.

    Include fdd_cpc.h right after fdd.h, both for the declaration and
    implementation.

    ## Functions

    ~~~C
    bool fdd_cpc_insert_dsk(fdd_t* fdd, const uint8_t* data, int data_size)
    ~~~
        'Inserts' a CPC .dsk disk image into the floppy drive.

        fdd         - pointer to an initialized fdd_t instance
        data        - pointer to the .dsk image data in memory
        data_size   - size in bytes of the image data

    ## zlib/libpng license

    Copyright (c) 2018 Andre Weissflog
    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.
        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.
        3. This notice may not be removed or altered from any source
        distribution. 
#*/
#ifdef __cplusplus
extern "C" {
#endif

/* load Amstrad CPC .dsk file format */
extern bool fdd_cpc_insert_dsk(fdd_t* fdd, const uint8_t* data, int data_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*--- IMPLEMENTATION ---------------------------------------------------------*/
#ifdef CHIPS_IMPL

/*
    load an Amstrad .DSK disc image

    http://www.cpcwiki.eu/index.php/Format:DSK_disk_image_file_format#Disk_image_file_format
*/
typedef struct {
    uint8_t magic[34];      /* MV - CPCEMU.... */
    uint8_t creator[14];
    uint8_t num_tracks;
    uint8_t num_sides;
    uint8_t track_size_l;
    uint8_t track_size_h;
    uint8_t pad[204];
} _fdd_cpc_dsk_header;

typedef struct {
    uint8_t magic[13];      /* Track-Info\r\n */
    uint8_t unused_0[3];
    uint8_t track_number;
    uint8_t side_number;
    uint8_t unused_1[2];
    uint8_t sector_size;
    uint8_t num_sectors;
    uint8_t gap_length;
    uint8_t filler_byte;
} _fdd_cpc_dsk_track_info;

typedef struct {
    uint8_t track;
    uint8_t side;
    uint8_t sector_id;
    uint8_t sector_size;    /* size_in_bytes = 0x80<<sector_size */
    uint8_t st1;
    uint8_t st2;
    uint8_t unused[2];
} _fdd_cpc_dsk_sector_info;

bool fdd_cpc_insert_dsk(fdd_t* fdd, const uint8_t* data, int data_size) {
    CHIPS_ASSERT(fdd);
    CHIPS_ASSERT(sizeof(_fdd_cpc_dsk_header) == 256);
    CHIPS_ASSERT(sizeof(_fdd_cpc_dsk_track_info) == 24);
    CHIPS_ASSERT(sizeof(_fdd_cpc_dsk_sector_info) == 8);
    CHIPS_ASSERT(data && (data_size > 0));
    if (fdd->has_disc) {
        fdd_eject_disc(fdd);
    }

    /* check if the header is valid */
    if (data_size > FDD_MAX_DISC_SIZE) {
        goto error;
    }
    if (data_size <= (int)sizeof(_fdd_cpc_dsk_header)) {
        goto error;
    }
    _fdd_cpc_dsk_header* hdr = (_fdd_cpc_dsk_header*) data;
    if (0 != memcmp(hdr->magic, "MV - CPC", 8)) {
        goto error;
    }
    if (hdr->num_sides > 2) {
        goto error;
    }
    if (hdr->num_tracks > FDD_MAX_TRACKS) {
        goto error;
    }
    const uint16_t track_size = (hdr->track_size_h<<8) | hdr->track_size_l;
    int file_size = hdr->num_sides * hdr->num_tracks * track_size + sizeof(_fdd_cpc_dsk_header);
    if (file_size != data_size) {
        goto error;
    }

    /* copy the data blob to the local buffer */
    fdd->data_size = data_size;
    memcpy(fdd->data, data, fdd->data_size);

    /* setup the disc structure */
    fdd_disc_t* disc = &fdd->disc;
    disc->formatted = true;
    disc->num_sides = hdr->num_sides;
    disc->num_tracks = hdr->num_tracks;
    int data_offset = sizeof(_fdd_cpc_dsk_header);
    for (int track_index = 0; track_index < disc->num_tracks; track_index++) {
        for (int side_index = 0; side_index < disc->num_sides; side_index++) {
            fdd_track_t* track = &disc->tracks[side_index][track_index];
            const _fdd_cpc_dsk_track_info* track_info = (const _fdd_cpc_dsk_track_info*) &fdd->data[data_offset];
            if (0 != memcmp("Track-Info\r\n", track_info->magic, sizeof(track_info->magic))) {
                goto error;
            }
            if ((data_offset + track_size) > data_size) {
                goto error;
            }
            track->data_offset = data_offset;
            track->data_size = track_size;
            track->num_sectors = track_info->num_sectors;
            const int sector_size = 0x80 << track_info->sector_size;
            if (sector_size != 512) {
                goto error;
            }
            int sector_data_offset = data_offset + 0x100;
            const _fdd_cpc_dsk_sector_info* sector_infos = (const _fdd_cpc_dsk_sector_info*) (track_info+1);
            for (int sector_index = 0; sector_index < track->num_sectors; sector_index++) {
                fdd_sector_t* sector = &track->sectors[sector_index];
                const _fdd_cpc_dsk_sector_info* sector_info = &sector_infos[sector_index];
                sector->info.upd765.c = sector_info->track;
                sector->info.upd765.h = sector_info->side;
                sector->info.upd765.r = sector_info->sector_id;
                sector->info.upd765.n = sector_info->sector_size;
                sector->info.upd765.st1 = sector_info->st1;
                sector->info.upd765.st2 = sector_info->st2;
                sector->data_offset = sector_data_offset;
                sector->data_size = sector_size;
                sector_data_offset += sector_size;
            }
            data_offset += 0x100 + track->num_sectors * sector_size;
            CHIPS_ASSERT(data_offset == sector_data_offset);
        }
    }
    fdd->has_disc = true;
    return true;
error:
    fdd_eject_disc(fdd);
    return false;
}
#endif /* CHIPS_IMPL */
