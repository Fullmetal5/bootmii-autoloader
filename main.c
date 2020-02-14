// Copyright 2008-2009  Segher Boessenkool  <segher@kernel.crashing.org>
// This code is licensed to you under the terms of the GNU GPL, version 2;
// see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

#include "loader.h"
#include "stub.h"


static u8 *const code_buffer = (u8 *)0x80040000;
static u8 *const trampoline_buffer = (u8 *)0x80001800;

static void dsp_reset(void)
{
	write16(0x0c00500a, read16(0x0c00500a) & ~0x01f8);
	write16(0x0c00500a, read16(0x0c00500a) | 0x0010);
	write16(0x0c005036, 0);
}

static u32 reboot_trampoline[] = {
	0x3c209300, // lis 1,0x9300
	0x60210000, // ori 1,1,0x0000
	0x7c2903a6, // mtctr 1
	0x4e800420  // bctr
};

int try_sd_load(void)
{
	int err;

	err = sd_init();
	if (err) {
		printf("SD card not found (%d)\n", err);
		return err;
	}

	err = fat_init();
	if (err == 0)
		printf("SD card detected\n");
	else {
		printf("SD card not detected (%d)\n", err);
		return err;
	}

//	if (usbgecko_checkgecko())
//		printf("USBGecko serial interface detected\n");
//	else
//		printf("USBGecko serial interface not detected\n");

	printf("Opening bootmii/armboot.bin:\n");
	err = fat_open("bootmii/armboot.bin");

	if (err) {
		printf("bootmii/armboot.bin not found (%d)\n", err);
		return err;
	}

extern u32 fat_file_size;

	printf("reading %d bytes...\n", fat_file_size);
	err = fat_read(code_buffer, fat_file_size);
	if (err) {
		printf("Error %d reading file\n", err);
		return err;
	}
	sync_after_write(code_buffer, fat_file_size);

	printf("Done.\n");
	return 0;
}

void __eabi(void){}

int main(void)
{
	dsp_reset();

	exception_init();

	// Install trampoline at 80001800; some payloads like to jump
	// there to restart.  Sometimes this can even work.
	memcpy(trampoline_buffer, reboot_trampoline, sizeof(reboot_trampoline));

	// Clear interrupt mask.
	write32(0x0c003004, 0);

	// Unlock EXI.
	write32(0x0d00643c, 0);

	video_init();
	usbgecko_init();

	printf("savezelda %s\n", version);
	printf("\n");
	printf("Copyright 2018-2020  Dexter Gerig\n");
	printf("Copyright 2008,2009  Segher Boessenkool\n");
	printf("Copyright 2008  Haxx Enterprises\n");
	printf("Copyright 2008  Hector Martin (\"marcan\")\n");
	printf("Copyright 2003,2004  Felix Domke\n");
	printf("\n");
	printf("This code is licensed to you under the terms of the\n");
	printf("GNU GPL, version 2; see the file COPYING\n");
	printf("\n");
	printf("Font and graphics by Freddy Leitner\n");
	printf("\n");
	printf("\n");

	printf("Cleaning up environment... ");

	reset_ios();

	printf("OK.\n");

	printf("Reloading to IOS80\n");
	int err = ios_reload(80);
	if (err) {
		printf("Fail, attempting to reload to IOS58\n");
		err = ios_reload(58);
		if (err) {
			printf("Sorry, the ES exploit is only tested on IOS80 or 58\n");
			goto hang;
		}
	}


	printf("Vuln discovery: metaconstruct\n");
	printf("Implementation: Fullmetal5\n");
	printf("Doing hax\n");

	err = try_sd_load();

	if (err) {
		printf("Couldn't load bootmii/armboot.bin\n");
		goto hang;
	}

	int es_hndl = ios_open("/dev/es", 0);
	printf("es_hndl: %d\n", es_hndl);

	STACK_ALIGN(struct ioctlv, hax, 2, 32);
	STACK_ALIGN(u32, a, 1, 32);
	*a = 0x00000000;

	printf("Preping call\n");
	hax[0].data = a;
	hax[0].len = 4;
	hax[1].data = phys_to_virt(0x201125b0);
	hax[1].len = 0;

	printf("Copying ES payload: 0x80010000:%x\n", stub_bin_len);
	memcpy(0x80010000, stub_bin, stub_bin_len);
	sync_after_write(0x80010000, stub_bin_len);

	printf("Launching armboot\n");
	int ret = ios_ioctlv(es_hndl, 0xf, 1, 1, hax);
	printf("Um, you shouldn't see this, something has gone wrong!\n");

	printf("ret: %d\n", ret);
hang:
	for (;;)
		;

	return 0;
}
