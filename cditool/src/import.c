#include "cditool.h"

#include <cdio/cdio.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline uint8_t byteToPBCD(uint8_t data)
{
    return ((data / 10) << 4) | (data % 10);
}

static inline void lsnToTime(const lsn_t lsn, uint8_t* minutes, uint8_t* seconds, uint8_t* sectors)
{
    *sectors = byteToPBCD(lsn % CDIO_CD_FRAMES_PER_SEC); // 75
    *seconds = byteToPBCD((lsn / CDIO_CD_FRAMES_PER_SEC) % CDIO_CD_SECS_PER_MIN); // 75 % 60
    *minutes = byteToPBCD(lsn / (CDIO_CD_FRAMES_PER_SEC * CDIO_CD_SECS_PER_MIN)); // 60
}

uint8_t header[16] = {
    0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0,
    0, 0, 0, 2,
};

/** \brief Import the CD-I disc from a CD reader.
 *
 * \param output The name of the output file.
 * \param drive The name of the CD drive. If NULL, the default one is used.
 * \return true if sucessful, false otherwise.
 */
int import(const char* output, const char* drive)
{
    if(output == NULL)
    {
        fprintf(stderr, "Error: no output file provided!\n");
        return false;
    }

    if(!cdio_init())
    {
        fprintf(stderr, "Error: could not initialize libcdio!\n");
        return false;
    }

    CdIo_t* cdi = cdio_open(drive, DRIVER_UNKNOWN);
    if(cdi == NULL)
    {
        fprintf(stderr, "Error: could not open drive %s\n", drive != NULL ? drive : "");
        return false;
    }

    if(cdio_get_discmode(cdi) != CDIO_DISC_MODE_CD_I)
        fprintf(stdout, "Warning: the disc is not recognized as a CD-I!\n");

    const lsn_t lastlsn = cdio_get_disc_last_lsn(cdi);
    const int size = 2352 * lastlsn;

    uint8_t* buf = malloc(size);
    if(buf == NULL)
    {
        fprintf(stderr, "Error: could not the required %d bytes!\n", size);
        cdio_destroy(cdi);
        return false;
    }

    FILE* out = fopen(output, "wb");
    if(out == NULL)
    {
        fprintf(stderr, "Error: could not open output file %s: %s\n", output, strerror(errno));
        cdio_destroy(cdi);
        free(buf);
        return false;
    }

    for(int lsn = 0, i = 0; lsn < lastlsn; lsn++, i += 2352)
    {
        lsnToTime(lsn + 150, &header[12], &header[13], &header[14]); // + 150 is for the inital 2 seconds offset.
        memcpy(&buf[i], header, 16);
        cdio_read_mode2_sector(cdi, &buf[i+16], lsn, 1);
    }

    fwrite(buf, 1, size, out);
    fclose(out);

    cdio_destroy(cdi);
    free(buf);
    return true;
}
