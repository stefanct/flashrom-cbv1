/*
 * sst39sf020.c: driver for SST28SF040C flash models.
 *
 *
 * Copyright 2000 Silicon Integrated System Corporation
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 * Reference:
 *	4 MEgabit (512K x 8) SuperFlash EEPROM, SST28SF040 data sheet
 *
 * $Id$
 */

#include "flash.h"
#include "jedec.h"

#define AUTO_PG_ERASE1		0x20
#define AUTO_PG_ERASE2		0xD0
#define AUTO_PGRM			0x10
#define CHIP_ERASE		0x30
#define RESET			0xFF
#define READ_ID			0x90

static __inline__ void protect_39sf020 (volatile char * bios)
{
	/* ask compiler not to optimize this */
	volatile unsigned char tmp;

	tmp = *(volatile unsigned char *) (bios + 0x1823);
	tmp = *(volatile unsigned char *) (bios + 0x1820);
	tmp = *(volatile unsigned char *) (bios + 0x1822);
	tmp = *(volatile unsigned char *) (bios + 0x0418);
	tmp = *(volatile unsigned char *) (bios + 0x041B);
	tmp = *(volatile unsigned char *) (bios + 0x0419);
	tmp = *(volatile unsigned char *) (bios + 0x040A);
}

static __inline__ void unprotect_39sf020 (volatile char * bios)
{
	/* ask compiler not to optimize this */
	volatile unsigned char tmp;

	tmp = *(volatile unsigned char *) (bios + 0x1823);
	tmp = *(volatile unsigned char *) (bios + 0x1820);
	tmp = *(volatile unsigned char *) (bios + 0x1822);
	tmp = *(volatile unsigned char *) (bios + 0x0418);
	tmp = *(volatile unsigned char *) (bios + 0x041B);
	tmp = *(volatile unsigned char *) (bios + 0x0419);
	tmp = *(volatile unsigned char *) (bios + 0x041A);
}

static __inline__ erase_sector_39sf020 (volatile char * bios, unsigned long address)
{
	*bios = AUTO_PG_ERASE1;
	*(bios + address) = AUTO_PG_ERASE2;

	/* wait for Toggle bit ready         */
	toggle_ready_jedec(bios);
}

static __inline__ write_sector_39sf020(volatile char * bios, unsigned char * src,
				       volatile unsigned char * dst, unsigned int page_size)
{
	int i;
	volatile char *Temp;

	for (i = 0; i < page_size; i++) {
		/* transfer data from source to destination */
		if (*src == 0xFF) {
			dst++, src++;
			/* If the data is 0xFF, don't program it */
			continue;
		}
            Temp =   (bios + 0x5555); 
            *Temp = 0xAA;                   
            Temp =  bios + 0x2AAA; 
            *Temp = 0x55; 
            Temp =  bios + 0x5555; 
            *Temp = 0xA0;                   
		*dst++ = *src++;
		toggle_ready_jedec(bios);
	}
}

int probe_39sf020 (struct flashchip * flash)
{
        volatile char * bios = flash->virt_addr;
        unsigned char  id1, id2;

        *(volatile char *) (bios + 0x5555) = 0xAA;
        myusec_delay(10);
        *(volatile char *) (bios + 0x2AAA) = 0x55;
        myusec_delay(10);
        *(volatile char *) (bios + 0x5555) = 0x90;

        myusec_delay(10);

        id1 = *(volatile unsigned char *) bios;
        id2 = *(volatile unsigned char *) (bios + 0x01);

        *(volatile char *) (bios + 0x5555) = 0xAA;
        *(volatile char *) (bios + 0x2AAA) = 0x55;
        *(volatile char *) (bios + 0x5555) = 0xF0;

        myusec_delay(10);

        printf(__FUNCTION__ "id1 %d, id2 %d\n", id1, id2);
        if (id1 == flash->manufacture_id && id2 == flash->model_id)
                return 1;

        return 0;
}

int erase_39sf020 (struct flashchip * flash)
{
	volatile char * bios = flash->virt_addr;

	unprotect_39sf020 (bios);
	*bios = CHIP_ERASE;
	*bios = CHIP_ERASE;
	protect_39sf020 (bios);

	myusec_delay(10);
	toggle_ready_jedec(bios);
}

int write_39sf020 (struct flashchip * flash, char * buf)
{
	int i;
	int total_size = flash->total_size * 1024, page_size = flash->page_size;
	volatile char * bios = flash->virt_addr;

//	unprotect_39sf020 (bios);

	printf ("Programming Page: ");
	for (i = 0; i < total_size/page_size; i++) {
		/* erase the page before programming */
		//erase_sector_39sf020(bios, i * page_size);

		/* write to the sector */
		printf ("%04d at address: 0x%08x", i, i * page_size);
		write_sector_39sf020(bios, buf + i * page_size, bios + i * page_size,
				     page_size);
		printf ("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	}
	printf("\n");

//	protect_39sf020 (bios);
}