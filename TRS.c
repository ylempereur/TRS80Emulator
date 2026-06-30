/* TRS.c */

/*————————————————————————————————————————————————————————————*/

#include <MacTypes.h>
#include <MixedMode.h>
#include <Events.h>
#include <Files.h>
#include <Resources.h>
#include <Errors.h>
#include <Processes.h>

/*————————————————————————————————————————————————————————————*/

typedef struct {
	Ptr diskBuff[2];
	long diskSize[2];
	short diskRefNum[2];
	Boolean diskProtect[2];
	Boolean diskDirty[2];
	Ptr screen;
	unsigned long driveTime;
	Byte driveSelect;
	Byte cassette;
} TRSRecord, *TRSPtr;

enum {
	uppTRSNewProcInfo = kD0DispatchedPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(TRSPtr)))
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short))),
	uppTRSDisposeProcInfo = kD0DispatchedPascalStackBased
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(TRSPtr))),
	uppTRSSwapROMProcInfo = kD0DispatchedPascalStackBased
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(TRSPtr)))
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short))),
	uppTRSResetProcInfo = kD0DispatchedPascalStackBased
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(TRSPtr))),
	uppTRSNMIProcInfo = kD0DispatchedPascalStackBased
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(TRSPtr))),
	uppTRSRunProcInfo = kD0DispatchedPascalStackBased
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(TRSPtr)))
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short))),
	uppTRSKeyDownProcInfo = kD0DispatchedPascalStackBased
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(TRSPtr)))
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(EventRecord *))),
	uppTRSKeyUpProcInfo = kD0DispatchedPascalStackBased
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(TRSPtr)))
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(EventRecord *)))
};

/*————————————————————————————————————————————————————————————*/

static UniversalProcPtr trsProc;

/*————————————————————————————————————————————————————————————*/

void TRSInit(void);
TRSPtr TRSNew(short romID);
void TRSDispose(TRSPtr this);
void TRSSwapROM(TRSPtr this, short romID);
void TRSReset(TRSPtr this);
void TRSNMI(TRSPtr this);
void TRSRun(TRSPtr this, short count);
void TRSKeyDown(TRSPtr this, const EventRecord *theEvent);
void TRSKeyUp(TRSPtr this, const EventRecord *theEvent);
Ptr TRSGetScreen(TRSPtr this);
Boolean TRSGetMode(TRSPtr this);
Byte TRSGetDriveSelect(TRSPtr this);
void TRSDrawText(const void *textBuf, short byteCount);
Boolean TRSHasDisk(TRSPtr this, short d);
OSErr TRSInsertDisk(TRSPtr this, short d, FSSpec *theDisk);
void TRSEjectDisk(TRSPtr this, short d);

/*————————————————————————————————————————————————————————————*/

void TRSInit(void)
{
	Handle resource;
	
	resource = Get1Resource('TRS ', 128);
	if (resource == nil)
		ExitToShell();
	HLockHi(resource);
	trsProc = (UniversalProcPtr) *resource;
}

/*————————————————————————————————————————————————————————————*/

TRSPtr TRSNew(short romID)
{
	return (TRSPtr) CallUniversalProc(trsProc, uppTRSNewProcInfo, 0, romID);
}

/*————————————————————————————————————————————————————————————*/

void TRSDispose(TRSPtr this)
{
	TRSEjectDisk(this, 0);
	TRSEjectDisk(this, 1);
	(void) CallUniversalProc(trsProc, uppTRSDisposeProcInfo, 1, this);
}

/*————————————————————————————————————————————————————————————*/

void TRSSwapROM(TRSPtr this, short romID)
{
	(void) CallUniversalProc(trsProc, uppTRSSwapROMProcInfo, 2, this, romID);
}

/*————————————————————————————————————————————————————————————*/

void TRSReset(TRSPtr this)
{
	(void) CallUniversalProc(trsProc, uppTRSResetProcInfo, 3, this);
}

/*————————————————————————————————————————————————————————————*/

void TRSNMI(TRSPtr this)
{
	(void) CallUniversalProc(trsProc, uppTRSNMIProcInfo, 4, this);
}

/*————————————————————————————————————————————————————————————*/

void TRSRun(TRSPtr this, short count)
{
	(void) CallUniversalProc(trsProc, uppTRSRunProcInfo, 5, this, count);
}

/*————————————————————————————————————————————————————————————*/

void TRSKeyDown(TRSPtr this, const EventRecord *theEvent)
{
	(void) CallUniversalProc(trsProc, uppTRSKeyDownProcInfo, 6, this, theEvent);
}

/*————————————————————————————————————————————————————————————*/

void TRSKeyUp(TRSPtr this, const EventRecord *theEvent)
{
	(void) CallUniversalProc(trsProc, uppTRSKeyUpProcInfo, 7, this, theEvent);
}

/*————————————————————————————————————————————————————————————*/

Ptr TRSGetScreen(TRSPtr this)
{
	return this->screen;
}

/*————————————————————————————————————————————————————————————*/

Boolean TRSGetMode(TRSPtr this)
{
	return (this->cassette & 0x08) != 0;
}

/*————————————————————————————————————————————————————————————*/

Byte TRSGetDriveSelect(TRSPtr this)
{
	if (this->driveSelect != 0x00 && TickCount() - this->driveTime >= 3*60)
		this->driveSelect = 0x00;
	return this->driveSelect;
}

/*————————————————————————————————————————————————————————————*/

void TRSDrawText(const void *textBuf, short byteCount)
{
	static Byte table[] = {
		0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
		0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
		0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
		0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
		0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
		0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
		0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
		0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
		0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
		0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
		0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
		0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
		0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
		0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
		0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
		0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF
	};
	Byte buffer[64];
	BytePtr p, q;
	short i;
	
	p = (BytePtr) textBuf, q = buffer;
	for (i = byteCount; i != 0; i--)
		*q++ = table[*p++];
	DrawText(buffer, 0, byteCount);
}

/*————————————————————————————————————————————————————————————*/

Boolean TRSHasDisk(TRSPtr this, short d)
{
	return this->diskSize[d] != 0;
}

/*————————————————————————————————————————————————————————————*/

OSErr TRSInsertDisk(TRSPtr this, short d, FSSpec *theDisk)
{
	HParamBlockRec pb;
	long count;
	OSErr error;
	
	TRSEjectDisk(this, d);
	pb.fileParam.ioNamePtr = theDisk->name;
	pb.fileParam.ioVRefNum = theDisk->vRefNum;
	pb.fileParam.ioFDirIndex = 0;
	pb.fileParam.ioDirID = theDisk->parID;
	error = PBHGetFInfoSync(&pb);
	if (error != noErr)
		return error;
	if (pb.fileParam.ioFlFndrInfo.fdType != 'MD80')
		return noErr;
	this->diskProtect[d] = (pb.fileParam.ioFlAttrib & 0x01) != 0;
	error = FSpOpenDF(theDisk, fsCurPerm, &this->diskRefNum[d]);
	if (error != noErr)
		return error;
	error = GetEOF(this->diskRefNum[d], &count);
	if (error != noErr) {
		(void) FSClose(this->diskRefNum[d]);
		return error;
	}
	if (count > 80*10*256) {
		(void) FSClose(this->diskRefNum[d]);
		return memFullErr;
	}
	error = FSRead(this->diskRefNum[d], &count, this->diskBuff[d]);
	if (error != noErr) {
		(void) FSClose(this->diskRefNum[d]);
		return error;
	}
	this->diskSize[d] = count;
	this->diskDirty[d] = false;
	return noErr;
}

/*————————————————————————————————————————————————————————————*/

void TRSEjectDisk(TRSPtr this, short d)
{
	long count;
	OSErr error;
	
	if (this->diskSize[d] == 0)
		return;
	if (this->diskDirty[d]) {
		error = SetFPos(this->diskRefNum[d], fsFromStart, 0);
		if (error == noErr) {
			count = this->diskSize[d];
			error = FSWrite(this->diskRefNum[d], &count, this->diskBuff[d]);
		}
	}
	error = FSClose(this->diskRefNum[d]);
	this->diskSize[d] = 0;
}

/*————————————————————————————————————————————————————————————*/
