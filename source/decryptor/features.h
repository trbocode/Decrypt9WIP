#pragma once

u32 SetNand(bool use_emunand);

u32 NcchPadgen(void);
u32 SdPadgen(void);
u32 CtrNandPadgen(void);
u32 TwlNandPadgen(void);

u32 DumpSeedsave(void);
u32 UpdateSeedDb(void);
u32 DumpTicket(void);
u32 DecryptTitlekeysFile(void);
u32 DecryptTitlekeysNand(void);

u32 DumpMovableSed(void);
u32 DumpSecureInfoA(void);
u32 DumpHealthAndSafety(void);
u32 InjectMovableSed(void);
u32 InjectSecureInfoA(void);
u32 InjectHealthAndSafety(void);

u32 DumpNand(void);
u32 DecryptAllNandPartitions(void);
u32 DecryptTwlNandPartition(void);
u32 DecryptCtrNandPartition(void);

u32 RestoreNand(void);
u32 InjectAllNandPartitions(void);
u32 InjectTwlNandPartition(void);
u32 InjectCtrNandPartition(void);

u32 DecryptSdFiles(void);
u32 DecryptNcsdNcch(void);
u32 DecryptCiaShallow(void);
u32 DecryptCiaDeep(void);
u32 EncryptNcsdNcch(void);
u32 EncryptCiaShallow(void);
