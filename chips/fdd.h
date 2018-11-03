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
#define FDD_MAX_TRACKS_SIZE (FDD_MAX_SECTORS*FDD_MAX_SECTOR_SIZE)
#define FDD_MAX_DISC_SIZE (FDD_MAX_SIDES*FDD_MAX_TRACKS*FDD_MAX_TRACK_SIZE)

/* UPD765 disc controller overlay of the sector info bytes */
typedef struct {
    uint8_t c;      /* cylinder number (track number) */
    uint8_t h;      /* head address (side) */
    uint8_t r;      /* record (sector number) */
    uint8_t n;      /* number (sector size) */
    uint8_t st0;    /* ST0 status register result */
    uint8_t st1;    /* ST1 status register result */
} fdd_upd765_info;

/* a sector description */
typedef struct {
    union {
        fdd_upd765_info upd765;
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
    bool formatted;     /* disc is formatted */
    bool protected;     /* disc is write protected */
    int num_sides;
    int num_tracks;
    fdd_track_t tracks[FDD_MAX_SIDES][FDD_MAX_TRACKS];
} fdd_disc_t;

/* a floppy disc drive description */
typedef struct {
    int cur_side;
    int cur_track;
    int cur_sector;
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

/* load Amstrad CPC .dsk file format */
extern bool fdd_insert_cpc_dsk(fdd_t* fdd, const uint_t* data, int data_size);

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

void fdd_eject(fdd_t* fdd) {
    CHIPS_ASSERT(fdd);
    fdd->cur_side = 0;
    fdd->cur_track = 0;
    fdd->cur_sector = 0;
    fdd->has_disc = false;
    fdd->motor_on = false;
    memset(&fdd->disc, 0, sizeof(fdd->disc));
    memset(&fdd->data, 0, sizeof(fdd->data));
}

bool _fdd_validate_disc(fdd_disc_t* disc) {
    CHIPS_ASSERT(disc);
    if ((disc->num_sides < 0) || (disc->num_sides > FDD_MAX_SIDES)) {
        return false;
    }
    if ((disc->num_tracks < 0) || (disc->num_tracks > FDD_MAX_TRACKS)) {
        return false;
    }
    for (int side_index = 0; side_index < disc->num_sides; side_index++) {
        for (int track_index = 0; track_index < disc->num_tracks; track_index++) {
            const fdd_track_t* track = &(fdd->tracks[side_index][track_index]);
            if (track->data_offset < 0) {
                return false;
            }
            if ((track->data_size < 0) || (fdd->data_size > FDD_MAX_TRACK_SIZE)) {
                return false;
            }
            if ((track->data_offset + track->data_size) > FDD_MAX_DISC_SIZE) {
                return false;
            }
            if ((track->num_sectors < 0) || (track->num_sectors > FDD_MAX_SECTORS)) {
                return false;
            }
            for (sector_index = 0; sector_index < track->num_sectors; sector_index++) {
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
    if (data)
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

/*
    load an Amstrad .DSK disc image */

    http://www.cpcwiki.eu/index.php/Format:DSK_disk_image_file_format#Disk_image_file_format
*/
bool fdd_insert_cpc_dsk(fdd_t* fdd, const uint_t* data, int data_size) {
    CHIPS_ASSERT(fdd);
    if (fdd->has_disc) {
        fdd_eject_disc(fdd);
    }

    /* 

    return true;
}

#endif /* CHIPS_IMPL */
