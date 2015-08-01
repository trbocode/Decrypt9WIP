#pragma once

u32 NcchPadgen(void);
u32 SdPadgen(void);
u32 CtrNandPadgen(void);
u32 TwlNandPadgen(void);
u32 DumpTicket(void);
u32 DecryptTitlekeysFile(void);
u32 DecryptTitlekeysNand(void);
u32 DumpNand(void);
u32 DecryptTwlAgbPartitions(void);
u32 DecryptCtrPartitions(void);
u32 DecryptTitles(void);

u32 RestoreNand(void);
u32 InjectTwlAgbPartitions(void);
u32 InjectCtrPartitions(void);
