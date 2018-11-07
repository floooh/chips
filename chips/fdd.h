#pragma once
/*#
    # fdd.h

    Floppy-disc drive helper functions.

    Do this:
    ~~~C
    #define CHIPS_IMPL
    ~~~
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provide the following macros with your own implementation
    
    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    FIXME: disk image loading code should probably go into separate headers
    FIXME: DOCS

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
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FDD_MAX_SIDES (2)           /* max number of disc sides */
#define FDD_MAX_TRACKS (80)         /* max number of tracks per side */
#define FDD_MAX_SECTORS (12)        /* max sectors per track */
#define FDD_MAX_SECTOR_SIZE (512)   /* max size of a sector in bytes */
#define FDD_MAX_TRACK_SIZE (FDD_MAX_SECTORS*FDD_MAX_SECTOR_SIZE)
#define FDD_MAX_DISC_SIZE (FDD_MAX_SIDES*FDD_MAX_TRACKS*FDD_MAX_TRACK_SIZE)

/* result bits (compatible with UPD765_RESULT_*) */
#define FDD_RESULT_SUCCESS (0)
#define FDD_RESULT_NOT_READY (1<<0)
#define FDD_RESULT_NOT_FOUND (1<<1)
#define FDD_RESULT_END_OF_SECTOR (1<<2)

/* UPD765 disc controller overlay of the sector info bytes */
typedef struct {
    uint8_t c;      /* cylinder number (track number) */
    uint8_t h;      /* head address (side) */
    uint8_t r;      /* record (sector number) */
    uint8_t n;      /* number (sector size) */
    uint8_t st1;    /* ST1 status register result */
    uint8_t st2;    /* ST2 status register result */
} fdd_upd765_info_t;

/* a sector description */
typedef struct {
    union {
        fdd_upd765_info_t upd765;
        uint8_t raw[8];
    } info;
    int data_offset;    /* start of sector data in disc data blob */
    int data_size;      /* size in bytes of sector data drive data buffer */
} fdd_sector_t;

/* a track description */
typedef struct {
    int data_offset;    /* offset of track data in disc data blob */
    int data_size;      /* track data size in bytes */
    int num_sectors;    /* number of sectors in track */
    fdd_sector_t sectors[FDD_MAX_SECTORS];  /* the sector descriptions */
} fdd_track_t;

/* a disc description */
typedef struct {
    bool formatted;         /* disc is formatted */
    bool write_protected;   /* disc is write protected */
    int num_sides;
    int num_tracks;
    fdd_track_t tracks[FDD_MAX_SIDES][FDD_MAX_TRACKS];
} fdd_disc_t;

/* a floppy disc drive description */
typedef struct {
    int cur_track_index;
    int cur_sector_index;
    int cur_sector_pos;
    bool has_disc;
    bool motor_on;
    fdd_disc_t disc;
    int data_size;
    uint8_t data[FDD_MAX_DISC_SIZE];
} fdd_t;

/* initialize a floppy disc drive */
extern void fdd_init(fdd_t* fdd);
/* drive motor on/off */
extern void fdd_motor(fdd_t* fdd, bool on);
/* insert a disc, the disc structure and data will be copied */
extern bool fdd_insert_disc(fdd_t* fdd, const fdd_disc_t* disc, const uint8_t* data, int data_size);
/* eject current disc */
extern void fdd_eject_disc(fdd_t* fdd);
/* seek to physical track (happens instantly), returns FDD_RESULT_* */
extern int fdd_seek_track(fdd_t* fdd, int track);
/* seek to sector on current physical track (happens instantly), returns FDD_RESULT_* */
extern int fdd_seek_sector(fdd_t* fdd, uint8_t c, uint8_t h, uint8_t r, uint8_t n);
/* read the next byte from the seeked-to sector, return FDD_RESULT_* */
extern int fdd_read(fdd_t* fdd, uint8_t h, uint8_t* out_data);

/* load Amstrad CPC .dsk file format */
extern bool fdd_insert_cpc_dsk(fdd_t* fdd, const uint8_t* data, int data_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*--- IMPLEMENTATION ---------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_DEBUG
    #ifdef _DEBUG
        #define CHIPS_DEBUG
    #endif
#endif
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

void fdd_init(fdd_t* fdd) {
    CHIPS_ASSERT(fdd);
    memset(fdd, 0, sizeof(fdd_t));
}

void fdd_motor(fdd_t* fdd, bool on) {
    CHIPS_ASSERT(fdd);
    fdd->motor_on = on;
}

void fdd_eject_disc(fdd_t* fdd) {
    CHIPS_ASSERT(fdd);
    fdd->cur_track_index = 0;
    fdd->cur_sector_index = 0;
    fdd->cur_sector_pos = 0;
    fdd->has_disc = false;
    fdd->motor_on = false;
    memset(&fdd->disc, 0, sizeof(fdd->disc));
    memset(&fdd->data, 0, sizeof(fdd->data));
}

bool _fdd_validate_disc(const fdd_disc_t* disc) {
    CHIPS_ASSERT(disc);
    if ((disc->num_sides < 0) || (disc->num_sides > FDD_MAX_SIDES)) {
        return false;
    }
    if ((disc->num_tracks < 0) || (disc->num_tracks > FDD_MAX_TRACKS)) {
        return false;
    }
    for (int side_index = 0; side_index < disc->num_sides; side_index++) {
        for (int track_index = 0; track_index < disc->num_tracks; track_index++) {
            const fdd_track_t* track = &(disc->tracks[side_index][track_index]);
            if (track->data_offset < 0) {
                return false;
            }
            if ((track->data_size < 0) || (track->data_size > FDD_MAX_TRACK_SIZE)) {
                return false;
            }
            if ((track->data_offset + track->data_size) > FDD_MAX_DISC_SIZE) {
                return false;
            }
            if ((track->num_sectors < 0) || (track->num_sectors > FDD_MAX_SECTORS)) {
                return false;
            }
            for (int sector_index = 0; sector_index < track->num_sectors; sector_index++) {
                const fdd_sector_t* sector = &(track->sectors[sector_index]);
                if (sector->data_offset < 0) {
                    return false;
                }
                if ((sector->data_size < 0) || (sector->data_size > FDD_MAX_SECTOR_SIZE)) {
                    return false;
                }
                if ((sector->data_offset + sector->data_size) > FDD_MAX_DISC_SIZE) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool fdd_insert_disc(fdd_t* fdd, const fdd_disc_t* disc, const uint8_t* data, int data_size) {
    CHIPS_ASSERT(fdd);
    if (fdd->has_disc) {
        fdd_eject_disc(fdd);
    }
    if (_fdd_validate_disc(disc)) {
        fdd->disc = *disc;
    }
    else {
        /* invalid disc structure */
        return false;
    }
    if (data) {
        if ((data_size > 0) && (data_size <= FDD_MAX_DISC_SIZE)) {
            fdd->data_size = data_size;
            memcpy(&fdd->data, data, data_size);
            fdd->disc.formatted = true;
        }
        else {
            /* invalid data size */
            return false;
        }
    }
    else {
        fdd->disc.formatted = false;
    }
    fdd->has_disc = true;
    return true;
}

int fdd_seek_track(fdd_t* fdd, int track) {
    CHIPS_ASSERT(fdd);
    if (fdd->has_disc && fdd->motor_on && (track < fdd->disc.num_tracks)) {
        fdd->cur_track_index = track;
        return FDD_RESULT_SUCCESS;
    }
    else {
        return FDD_RESULT_NOT_READY;
    }
}

int fdd_seek_sector(fdd_t* fdd, uint8_t c, uint8_t h, uint8_t r, uint8_t n) {
    CHIPS_ASSERT(fdd);
    CHIPS_ASSERT(h < FDD_MAX_SIDES);
    if (fdd->has_disc && fdd->motor_on) {
        const fdd_track_t* track = &fdd->disc.tracks[h][fdd->cur_track_index];
        for (int si = 0; si < track->num_sectors; si++) {
            const fdd_sector_t* sector = &track->sectors[si];
            if (sector->info.upd765.r == r) {
                fdd->cur_sector_index = si;
                fdd->cur_sector_pos = 0;
                return FDD_RESULT_SUCCESS;
            }
        }
        return FDD_RESULT_NOT_FOUND;
    }
    else {
        return FDD_RESULT_NOT_READY;
    }
}

int fdd_read(fdd_t* fdd, uint8_t h, uint8_t* out_data) {
    CHIPS_ASSERT(fdd && (h < FDD_MAX_SIDES) && out_data);
    if (fdd->has_disc & fdd->motor_on) {
        const fdd_sector_t* sector = &fdd->disc.tracks[h][fdd->cur_track_index].sectors[fdd->cur_sector_index];
        if (fdd->cur_sector_pos < sector->data_size) {
            const int data_offset = sector->data_offset + fdd->cur_sector_pos;
            *out_data = fdd->data[data_offset];
            fdd->cur_sector_pos++;
            if (fdd->cur_sector_pos < sector->data_size) {
                return FDD_RESULT_SUCCESS;
            }
            else {
                return FDD_RESULT_END_OF_SECTOR;
            }
        }
        return FDD_RESULT_NOT_FOUND;
    }
    out_data = 0xFF;
    return FDD_RESULT_NOT_READY;
}

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

bool fdd_insert_cpc_dsk(fdd_t* fdd, const uint8_t* data, int data_size) {
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
