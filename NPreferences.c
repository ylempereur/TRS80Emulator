/* NPreferences.c */

/*————————————————————————————————————————————————————————————*/

#include <MacTypes.h>
#include <ControlDefinitions.h>
#include <Dialogs.h>
#include <NumberFormatting.h>
#include <Sound.h>
#include <Resources.h>

#include "TRS80.h"

/*————————————————————————————————————————————————————————————*/

#define theDialogID 128

/*————————————————————————————————————————————————————————————*/

typedef struct {
	DialogPtr dialog;
} PDRecord, *PDPtr;

/*————————————————————————————————————————————————————————————*/

extern short gRate;

/*————————————————————————————————————————————————————————————*/

extern void DoUpdate(const EventRecord *theEvent);
extern void DoActivate(const EventRecord *theEvent);
extern void SetCursorID(short cursorID);

/*————————————————————————————————————————————————————————————*/

void NPreferences(void);
static pascal Boolean MyFilter(DialogPtr dialog, EventRecord *event, SInt16 *itemHit);
static pascal ControlKeyFilterResult MyKeyFilter(ControlHandle control, SInt16 *keyCode,
		SInt16 *charCode, SInt16 *modifiers);
static void AdjustCursor(PDPtr this);
static void AdjustOK(PDPtr this);
static Boolean IsOKActive(PDPtr this);
static void ActivateOK(PDPtr this);
static void DeactivateOK(PDPtr this);

static void CenterDialog(short dialogID);
static void FlashButton(DialogPtr dialog, SInt16 itemNo);
static void NSetDialogItemData(DialogPtr dialog, SInt16 itemNo, ControlPartCode part,
		ResType tagName, Size size, Ptr data);
static void NGetDialogItemText(DialogPtr dialog, SInt16 itemNo, Str255 text, SInt16 maxLen);
static void NSetDialogItemText(DialogPtr dialog, SInt16 itemNo, ConstStr255Param text);

/*————————————————————————————————————————————————————————————*/

void NPreferences(void)
{
	register PDPtr this;
	Str15 string;
	GrafPtr savePort;
	ControlKeyFilterUPP myKeyFilter;
	ModalFilterUPP myFilter;
	long num;
	OSErr error;
	short itemHit;
	Boolean dfltRing;
	
	this = (PDPtr) NewPtr(sizeof(PDRecord));
	if (this == nil)
		return;
	SetCursorID(watchCursor);
	CenterDialog(theDialogID);
	this->dialog = GetNewDialog(theDialogID, nil, (WindowPtr) -1);
	if (this->dialog == nil) {
		DisposePtr((Ptr) this);
		return;
	}
	GetPort(&savePort);
	SetPort(this->dialog);
	SetWRefCon(this->dialog, (long) this);
	dfltRing = true;
	NSetDialogItemData(this->dialog, ok, kControlNoPart, kControlPushButtonDefaultTag,
			sizeof dfltRing, (Ptr) &dfltRing);
	myKeyFilter = NewControlKeyFilterProc(MyKeyFilter);
	NSetDialogItemData(this->dialog, 3, kControlNoPart, kControlEditTextKeyFilterTag,
			sizeof myKeyFilter, (Ptr) &myKeyFilter);
	NumToString(gRate, string);
	NSetDialogItemText(this->dialog, 3, string);
	AdjustOK(this);
	myFilter = NewModalFilterProc(MyFilter);
	ShowWindow(this->dialog);
	SetCursorID(arrowCursor);
	error = 1;
	do {
		ModalDialog(myFilter, &itemHit);
		switch (itemHit) {
			case ok:
				NGetDialogItemText(this->dialog, 3, string, 15);
				StringToNum(string, &num);
				gRate = num;
				error = noErr;
				break;
			case cancel:
				error = userCanceledErr;
				break;
			case 3:
				AdjustOK(this);
				break;
		}
	} while (error == 1);
	SetCursorID(watchCursor);
	HideWindow(this->dialog);
	DisposeRoutineDescriptor(myFilter);
	DisposeRoutineDescriptor(myKeyFilter);
	SetPort(savePort);
	DisposeDialog(this->dialog);
	DisposePtr((Ptr) this);
}

/*————————————————————————————————————————————————————————————*/

static pascal Boolean MyFilter(DialogPtr dialog, EventRecord *event, SInt16 *itemHit)
{
	register PDPtr this;
	short keyCode, charCode, modifiers;
	
	this = (PDPtr) GetWRefCon(dialog);
	AdjustCursor(this);
	switch (event->what) {
		case nullEvent:
			break;
		case activateEvt:
			if ((WindowPtr) event->message != this->dialog) {
				DoActivate(event);
				event->what = nullEvent;
			}
			break;
		case updateEvt:
			if ((WindowPtr) event->message != this->dialog) {
				DoUpdate(event);
				event->what = nullEvent;
			}
			break;
		case keyDown:
		case autoKey:
			keyCode = event->message;
			charCode = keyCode & charCodeMask;
			modifiers = event->modifiers;
			if (charCode == CR || charCode == ETX) {
				if (IsOKActive(this)) {
					FlashButton(this->dialog, ok);
					*itemHit = ok;
					return true;
				}
				event->what = nullEvent;
				break;
			}
			if (keyCode == 0x351B || charCode == '.' && (modifiers & cmdKey) != 0) {
				FlashButton(this->dialog, cancel);
				*itemHit = cancel;
				return true;
			}
			if ((modifiers & cmdKey) != 0) {
				if (charCode == 'a') {
					SelectDialogItemText(this->dialog, 3, 0, 32767);
					event->what = nullEvent;
					break;
				}
				if (charCode == 'x' || charCode == 'c' || charCode == 'v')
					break;
				SysBeep(30);
				event->what = nullEvent;
				break;
			}
			if (charCode == HT)
				break;
			if (charCode == ESC) {
				charCode = BS;
				event->message = charCode;
			}
			if (charCode < ' ' || charCode == DEL) {
				if (charCode == BS || charCode >= FS && charCode <= US)
					break;
				SysBeep(30);
				event->what = nullEvent;
				break;
			}
			break;
	}
	return false;
}

/*————————————————————————————————————————————————————————————*/

static pascal ControlKeyFilterResult MyKeyFilter(ControlHandle control, SInt16 *keyCode,
		SInt16 *charCode, SInt16 *modifiers)
{
#if GENERATINGPOWERPC
#pragma unused (keyCode)
#endif
	
	TEHandle hTE;
	TEPtr pTE;
	Size actualSize;
	
	if ((*modifiers & cmdKey) != 0) {
		if (*charCode == 'x' || *charCode == 'c')
			return kControlKeyFilterPassKey;
		SysBeep(30);
		return kControlKeyFilterBlockKey;
	}
	if (*charCode == BS || *charCode >= FS && *charCode <= US)
		return kControlKeyFilterPassKey;
	if (*charCode < '0' || *charCode > '9') {
		SysBeep(30);
		return kControlKeyFilterBlockKey;
	}
	GetControlData(control, kControlNoPart, kControlEditTextTEHandleTag, sizeof hTE, (Ptr) &hTE,
			&actualSize);
	pTE = *hTE;
	if (pTE->teLength + pTE->selStart - pTE->selEnd + 1 > 4) {
		SysBeep(30);
		return kControlKeyFilterBlockKey;
	}
	return kControlKeyFilterPassKey;
}

/*————————————————————————————————————————————————————————————*/

static void AdjustCursor(PDPtr this)
{
	Point mouseLoc;
	short itemHit;
	
	GetMouse(&mouseLoc);
	itemHit = FindDialogItem(this->dialog, mouseLoc) + 1;
	switch (itemHit) {
		case 3:
			SetCursorID(iBeamCursor);
			break;
		default:
			SetCursorID(arrowCursor);
			break;
	}
}

/*————————————————————————————————————————————————————————————*/

static void AdjustOK(PDPtr this)
{
	Str15 string;
	long num;
	
	NGetDialogItemText(this->dialog, 3, string, 15);
	if (StrLength(string) == 0) {
		DeactivateOK(this);
		return;
	}
	StringToNum(string, &num);
	if (num < 1 || num > 9999) {
		DeactivateOK(this);
		return;
	}
	ActivateOK(this);
}

/*————————————————————————————————————————————————————————————*/

static Boolean IsOKActive(PDPtr this)
{
	ControlHandle control;
	
	GetDialogItemAsControl(this->dialog, ok, &control);
	return IsControlActive(control);
}

/*————————————————————————————————————————————————————————————*/

static void ActivateOK(PDPtr this)
{
	ControlHandle control;
	Boolean active;
	
	GetDialogItemAsControl(this->dialog, ok, &control);
	active = IsControlActive(control);
	if (active)
		return;
	ActivateControl(control);
}

/*————————————————————————————————————————————————————————————*/

static void DeactivateOK(PDPtr this)
{
	ControlHandle control;
	Boolean active;
	
	GetDialogItemAsControl(this->dialog, ok, &control);
	active = IsControlActive(control);
	if (!active)
		return;
	DeactivateControl(control);
}

/*————————————————————————————————————————————————————————————*/

static void CenterDialog(short dialogID)
{
	DialogTHndl dialog;
	DialogTPtr d;
	short dh, dv;
	
	dialog = (DialogTHndl) GetResource('DLOG', dialogID);
	if (dialog == nil)
		return;
	HNoPurge((Handle) dialog);
	d = *dialog;
	dv = ((GetMBarHeight() + qd.screenBits.bounds.top - d->boundsRect.top) * 2
			+ qd.screenBits.bounds.bottom - d->boundsRect.bottom) / 3;
	dh = (qd.screenBits.bounds.left + qd.screenBits.bounds.right
			- d->boundsRect.left - d->boundsRect.right) / 2;
	OffsetRect(&d->boundsRect, dh, dv);
}

/*————————————————————————————————————————————————————————————*/

static void FlashButton(DialogPtr dialog, SInt16 itemNo)
{
	ControlHandle control;
	unsigned long finalTicks;
	
	GetDialogItemAsControl(dialog, itemNo, &control);
	HiliteControl(control, kControlButtonPart);
	Delay(8, &finalTicks);
	HiliteControl(control, kControlNoPart);
}

/*————————————————————————————————————————————————————————————*/

static void NSetDialogItemData(DialogPtr dialog, SInt16 itemNo, ControlPartCode part,
		ResType tagName, Size size, Ptr data)
{
	ControlHandle control;
	
	GetDialogItemAsControl(dialog, itemNo, &control);
	SetControlData(control, part, tagName, size, data);
}

/*————————————————————————————————————————————————————————————*/

static void NGetDialogItemText(DialogPtr dialog, SInt16 itemNo, Str255 text, SInt16 maxLen)
{
	register BytePtr p, q;
	Str255 tempText;
	ControlHandle control;
	short length;
	
	GetDialogItemAsControl(dialog, itemNo, &control);
	GetDialogItemText((Handle) control, tempText);
	p = tempText;
	length = *p++;
	if (length > maxLen)
		length = maxLen;
	q = text;
	*q++ = length;
	BlockMoveData(p, q, length);
}

/*————————————————————————————————————————————————————————————*/

static void NSetDialogItemText(DialogPtr dialog, SInt16 itemNo, ConstStr255Param text)
{
	ControlHandle control;
	
	GetDialogItemAsControl(dialog, itemNo, &control);
	SetDialogItemText((Handle) control, text);
}

/*————————————————————————————————————————————————————————————*/
