#pragma once

// Message-Id für Thread -> View-Kommunikation
#define ADSB_DATA				(WM_USER+1)
#define ADSB_THREAD_FINISHED_UNEXPECTED	(WM_USER+2)

// erwartete Anzahl von ListCtrl-Items
#define ITEM_COUNT 50000

// Länge der ADSB Messages in Bits
#define ADSB_LONG_MESSAGE_BITS 112
#define ADSB_SHORT_MESSAGE_BITS 56
