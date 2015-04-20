#define CRCTableSize 74
const char *UCodeNames[15] =   {"Microcode Autodetection failed (UCode CRC: 0x%X)",
								"Detected Microcode 1 - RSP SW Version 2.0x",
								"Detected Microcode 2 - F3DEX series",
								"Detected Microcode 3 - F3DEX2 series",
								"Detected Microcode 4 - WaveRace US",
								"Detected Microcode 5 - Diddy Kong Racing",
								"Detected unsupported ucode",
								"Detected Microcode 7 - Perfect Dark",
								"Detected Microcode 8 - F3DEX2 with Conker's BFD extensions",
								"Detected Microcode 9 - Battle for Naboo",
								"Detected Microcode 10 - S2DEX",
								"Detected Microcode 11 - Jet Force Gemini",
								"Detected Microcode 12 - Rogue Squadron",
								"Detected Microcode 13 - Resident Evil II",
								"Detected Microcode 14 - S2DEX2"};

bool UCode_NoN;

const struct {
	U32 CRC;
	U32 ID;
	bool NoN;
} CRCTable[CRCTableSize] = {

	0x7FE32C72, 1, false, // RSP SW Version: 2.0D, 04-01-96        Super Mario
	0x6F0064B6, 1, false, // RSP SW Version: 2.0D, 04-01-96        Pilot Wings 64
	0xC2F0F0FF, 1, false, // RSP SW Version: 2.0D, 04-01-96        Dark Rift
	0x8B7E9B30, 1, false, // RSP SW Version: 2.0D, 04-01-96        Killer Instinct Gold
	0xE1752677, 1, false, // RSP SW Version: 2.0D, 04-01-96        Star Wars: Shadows of the Empire
	0xE17526BB, 4, false, // RSP SW Version: 2.0D, 04-01-96        WaveRace US
	0x5FE54F16, 1, false, // RSP SW Version: 2.0G, 09-30-96		   Aero Fighters Assault	
	0x86CBEAC5, 1, false, // RSP SW Version: 2.0G, 09-30-96        Goldeneye
	0xB53D4DCD, 1, false, // RSP SW Version: 2.0G, 09-30-96        Mame64, most likely all demos
	0xF5B097E5, 1, false, // RSP SW Version: 2.0H, 02-12-97        Duke Nukem 64, Jeopardy
	0x22D33C08, 1, false, // RSP SW Version: 2.0H, 02-12-97        Tetrisphere
	0x3950D13E, 1, false, // RSP SW Version: 2.0H, 02-12-97        Midway's Greatest Arcade Hits

	0xB483AAC2, 2, false, // RSP Gfx ucode F3DEX         0.95      Mario Kart
	0x48E72211, 2, false, // RSP Gfx ucode F3DEX.NoN     1.00      Doom64
	0x0D99A611, 2, false, // RSP Gfx ucode F3DEX.NoN     1.00      ExtremeG
	0x40CFF45E, 2, false, // RSP Gfx ucode F3DEX.NoN     1.21      Fighter's Destiny
	0x597FD8A5, 2, false, // RSP Gfx ucode F3DEX.NoN     1.21      Tamiya Racing
	0x2B4F66E0, 2, false, // RSP Gfx ucode F3DEX         1.21      Banjo Kazooie, Biofreaks (and many others)
	0x93336046, 2, false, // RSP Gfx ucode F3DEX         1.21      Tetris 64
	0x6D61FBF2, 2, false, // RSP Gfx ucode F3DEX         1.21      S.F. Rush
	0xF54BAC08, 2, false, // RSP Gfx ucode F3DLX         1.21      Wipeout
	0x6D6574CC, 2, false, // RSP Gfx ucode F3DLX.Rej     1.21      Mortal Kombat Mythologies
	0x16E55476, 2, false, // RSP Gfx ucode F3DEX.NoN     1.22      Starfox
	0x476E9FEC, 2, false, // RSP Gfx ucode F3DEX         1.23      Aerogauge
	0xC69043A6, 2, false, // RSP Gfx ucode F3DEX         1.23      A Bug's life
	0xC455F2B9, 2, false, // RSP Gfx ucode F3DEX         1.23      Penny racers
	0x38663CD7, 2, false, // RSP Gfx ucode F3DEX         1.23      Forsaken
	0xBEA3A966, 2, false, // RSP Gfx ucode F3DEX         1.23      Quake II
	0x2498CAC2, 2, false, // RSP Gfx ucode F3DLX.Rej     1.23      Puzzle Bobble 64
	0xA789A25A, 2, false, // RSP Gfx ucode F3DTEX/A      1.23      Tamagotchi World

	0x837C663B, 2, false, // RSP Gfx ucode S2DEX         1.06      Yoshi's Story - F3DEX part
	0x07C0B2A7, 10,false, // RSP Gfx ucode S2DEX         1.06      Yoshi's Story (Initial ucode is S2DEX - EURO VERSION), 
	0x7FEFECF2, 10,false, // RSP Gfx ucode S2DEX         1.06      Yoshi's Story (Initial ucode is S2DEX)
	0xCFE0A1B0, 10,false, // RSP Gfx ucode S2DEX         1.06      Yoshi's Story - S2DEX part
	0xE1C96429, 10,false, // RSP Gfx ucode S2DEX         1.07      Starcraft 64

	0xDEE825BF, 3, false, // RSP Gfx ucode F3DEX.NoN   fifo 2.03   F-Zero-X
	0xDEE845FF, 3, false, // RSP Gfx ucode F3DEX.NoN   fifo 2.03   Starshot - Space Circus Fever
	0x1EBC8E93, 3, false, // RSP Gfx ucode F3DEX       fifo 2.04   Buck Bumble 
	0x1EBCAED3, 3, false, // RSP Gfx ucode F3DEX.NoN   fifo 2.04   Monster Truck Madness
	0x4DC61231, 3, false, // RSP Gfx ucode F3DEX.NoN   fifo 2.04H  Kirby 64
	0x8BCE721E, 3, false, // RSP Gfx ucode F3DLX.Rej   fifo 2.05   All-Star Baseball
	0x755EEE6B, 3, false, // RSP Gfx ucode F3DEX       fifo 2.05   Snowboard Kids 2
	0x17193213, 3, false, // RSP Gfx ucode F3DEX       fifo 2.05   F-Zero-X(E)/Chameleon Twist 2
	0x2F5303F1, 3, false, // RSP Gfx ucode F3DEX.NoN   fifo 2.05   Armorines Project Swarm
	0x17214213, 3, false, // RSP Gfx ucode F3DEX.NoN   fifo 2.05   Southpark Chef Luv Shack
	0x14D4F478, 3, false, // RSP Gfx ucode F3DEX.NoN   fifo 2.06   Castlevania (1&2), Pokemon Stadium
	0x14F53478, 3, false, // RSP Gfx ucode F3DEX.NoN   fifo 2.06   Twisted Edge
	0xAD4840BB, 3, false, // RSP Gfx ucode F3DEX       fifo 2.06   Resident Evil II (has several ucode descriptions, including ID=2 ones)
	0xFEFDB476, 3, true,  // RSP Gfx ucode F3DZEX.NoN  fifo 2.06H  Zelda
	0xD2F25057, 3, false, // RSP Gfx ucode F3DEX.NoN   fifo 2.07   Beetle Adventure Racing
	0xD2B21057, 3, false, // RSP Gfx ucode F3DEX.NoN   fifo 2.07   Destruction Derby, Donkey Kong 64
	0x642202AE, 3, false, // RSP Gfx ucode F3DEX       xbus 2.07   Lode Runner 3-D
	0x40AB9AF2, 3, false, // RSP Gfx ucode F3DEX.NoN   fifo 2.08   Ridge Racer
	0x41AC9AF2, 3, false, // RSP Gfx ucode F3DEX.NoN   fifo 2.08   Banjo Tooie
	0x6A64FC07, 3, false, // RSP Gfx ucode F3DEX.NoN   fifo 2.08   Excitebike 64
	0x4F27E0FD, 3, true,  // RSP Gfx ucode F3DEX.NoN   fifo 2.08   Paper Mario
	0x379848EF, 3, false, // RSP Gfx ucode F3DEX.NoN   fifo 2.08H  Pokemon Snap
	0xFC7C75EC, 3, false, // RSP Gfx ucode F3DEX       fifo 2.08   Tony Hawk
	0xF76AD08A, 3, false, // RSP Gfx ucode F3DEX       fifo 2.08   Cruis'n Exotica
	0xFC7C35AC, 3, false, // RSP Gfx ucode F3DEX       xbus 2.08   Southpark Rally
	0xF8AE0B39, 8, true,  // RSP Gfx ucode F3DEXBG.NoN fifo 2.08   Conker's Bad Fur Day
	0x4F3C60FB, 3, true,  // RSP Gfx ucode F3DZEX.NoN  fifo 2.08I  Zelda MoM
	0x4F2BE0FD, 3, false, // RSP Gfx ucode F3DZEX.NoN  fifo 2.08J  Doubutsu no Mori

	0xA2857197, 14,false, // RSP Gfx ucode S2DEX       fifo 2.08   Worms N64/Evangelion

	0x5614A9FA, 3, false, // RSP Gfx ucode L3DEX       fifo 2.08   Hey you, Pikachu

	0xC29D807D, 5, false, // ????????????????????????????????????  Diddy Kong Racing
	0xFEC47C05, 5, false, // ????????????????????????????????????  Diddy Kong Racing
	0xF6061B18, 11,false, // ????????????????????????????????????  Jet Force Gemini
	0xC89C8B19, 7, false, // ????????????????????????????????????  Perfect Dark
	0xEB19F1A4, 9, false, // ????????????????????????????????????  Star Wars: Rogue Squadron
	0x1EC35AC0, 12,false, // ????????????????????????????????????  Star Wars: Battle for Naboo
	0x5A4AF0CC, 6, false, // ????????????????????????????????????  Indiana Jones
	0x7B83177E, 6, false, // ????????????????????????????????????  Cube demo
	0xE854877A, 6, false // ????????????????????????????????????  World Driver Championship
};

U32 UCodeCRC;

__inline U32 MicroCodeDetect(U8 *RDRAM, U32 StartRDRAM)
{
	// UCode CRC (assume size=3072 bytes)
	U32 i;
	U32 Val;
	UCodeCRC = 0;
	for (i=0;i<=(3072>>2);i++){
		Val = *((U32*)&RDRAM[StartRDRAM+(i<<2)]);
		__asm{
			mov eax, Val;
			mov ecx, i;
			rol eax, cl;
			add UCodeCRC, eax;
		}
	}
	for (i=0;i<CRCTableSize;i++){
		if (CRCTable[i].CRC == UCodeCRC){
			UCode_NoN = CRCTable[i].NoN;
			return CRCTable[i].ID;
		}
	}
	return 0;
}
