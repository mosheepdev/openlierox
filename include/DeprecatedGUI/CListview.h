/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// gui list view class
// Created 30/5/02
// Jason Boettcher


#ifndef __CLISTVIEW_H__DEPRECATED_GUI__
#define __CLISTVIEW_H__DEPRECATED_GUI__

#include <string>
#include "DeprecatedGUI/CWidget.h"
#include "DeprecatedGUI/CScrollbar.h"

namespace DeprecatedGUI {

// Event msg's for this control
enum {
	LV_NONE=-1,
	LV_CHANGED=0,
    LV_RIGHTCLK,
	LV_DOUBLECLK,
	LV_DELETE,
	LV_ENTER,
	LV_WIDGETEVENT
};


// Listview messages
enum {
	LVS_ADDCOLUMN=0,
	LVS_ADDITEM,
	LVM_REMOVEITEM,
	LVM_GETCURINDEX,
	LVS_GETCURSINDEX,
	LVM_GETITEMCOUNT,
	LVM_CLEAR,
	LVM_GETITEMINDEX,
	LVM_GETINDEX,
    LVM_GETCURITEM,
	LVM_SETOLDSTYLE,
	LVM_GETCOLUMNWIDTH
};


// Sub-item types
enum {
	LVS_IMAGE=0,
	LVS_TEXT,
	LVS_WIDGET
};

// Subitem vertical aligns
enum  {
	VALIGN_TOP=0,
	VALIGN_MIDDLE,
	VALIGN_BOTTOM
};



// Column structure
class lv_column_t { public:
	std::string	sText;
	int			iWidth;
	bool		bDown;
	int			iSorted; // -1 = unsorted, 0 = descending, 1 = ascending
	Uint32		iColour;

	lv_column_t *tNext;

};


// Sub item structure
class lv_subitem_t { public:
	lv_subitem_t() : tWidget(NULL) {} // safety
	~lv_subitem_t()  { if (tWidget) delete tWidget; }

	int			iType;
	std::string	sText;
	SmartPointer<SDL_Surface> bmpImage;
	CWidget		*tWidget;
	bool		bVisible;
	int			iExtra;
	int			iValign;
	Uint32		iColour;
	Uint32		iBgColour;
	Uint8		iBgAlpha;

	lv_subitem_t *tNext;

};


// Item structure
class lv_item_t { public:
	std::string	sIndex;
	int			iIndex;
    int         _iID;
	bool		bSelected;
	int			iHeight;
	Uint32		iColour;
	Uint32		iBgColour;
	Uint8		iBgAlpha;

	lv_subitem_t *tSubitems;

	lv_item_t	*tNext;

};

// Listview control class
class CListview: public CWidget {
public:
	// Constructors
	CListview() {
		tColumns = NULL;
		iNumColumns = 0;
		tItems = NULL;
		tLastItem = NULL;
		tSelected = NULL;
		iItemCount=0;
		bGotScrollbar = false;
		iType = wid_Listview;
		fLastMouseUp = -99999;
		iContentHeight = 0;
        iItemID = 0;
        bShowSelect = true;
		iLastMouseX = 0;
		iGrabbed = 0;
		bOldStyle = false;
		iSavedScrollbarPos = 0;
		iLastChar = 0;
		bDrawBorder = true;
		bNeedsRepaint = true;
		bCustomScrollbarSetup = false;
		bAlwaysVisibleScrollbar = false;
		tFocusedSubWidget = NULL;
		tMouseOverSubWidget = NULL;
	}

	~CListview() {
		Destroy();
	}


private:
	// Attributes
	bool			bOldStyle;
	bool			bDrawBorder;
	bool            bShowSelect;
	bool			bCustomScrollbarSetup;
	bool			bAlwaysVisibleScrollbar;

	// Columns
	int				iNumColumns;
	lv_column_t		*tColumns;
	int				iGrabbed;
	int				iLastMouseX;

	// Items
	lv_item_t		*tItems;
	lv_item_t		*tLastItem;
	lv_item_t		*tSelected;
	int				iItemCount;
    int             iItemID;
	int				iContentHeight;

	float			fLastMouseUp;
	int				iClickedSub;

	// Scrollbar
	CScrollbar		cScrollbar;
	bool			bGotScrollbar;
	int				iSavedScrollbarPos;

	// Other
	bool			bNeedsRepaint;
	UnicodeChar		iLastChar;
	gui_event_t		tLastWidgetEvent;
	CWidget			*tFocusedSubWidget;
	CWidget			*tMouseOverSubWidget;

public:
	// Methods

	void	Create(void);
	void	Destroy(void);

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse);
	int		MouseUp(mouse_t *tMouse, int nDown);
	int		MouseDown(mouse_t *tMouse, int nDown);
	int		MouseWheelDown(mouse_t *tMouse);
	int		MouseWheelUp(mouse_t *tMouse);
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate);

	void	Draw(SDL_Surface * bmpDest);

	void	LoadStyle(void) {}

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2);
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param);
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param);

	void	ReadjustScrollbar(void);
	void	SetupScrollbar(int x, int y, int h, bool always_visible);

	void	Clear(void);

	void	SortBy(int column, bool ascending);
	void	ReSort(void);

	void	AddColumn(const std::string& sText, int iWidth);
	void	AddColumn(const std::string& sText, int iWidth, Uint32 iColour);
	lv_item_t* AddItem(const std::string& sIndex, int iIndex, int iColour);
	void	AddSubitem(int iType, const std::string& sText, const SmartPointer<SDL_Surface> & img, CWidget *wid, int iVAlign = VALIGN_MIDDLE);
	void	AddSubitem(int iType, const std::string& sText, const SmartPointer<SDL_Surface> & img, CWidget *wid, int iVAlign, Uint32 iColour);

	void	RemoveItem(int iIndex);
	int		getIndex(int count);

	int		GetColumnWidth(int id);

    int     getSelectedID(void);
    void    setSelectedID(int id);

	void	setOldStyle(bool _s)	{ bOldStyle = _s; }
	bool	getOldStyle(void)		{ return bOldStyle; }

    int     getNumItems(void)	{ return iItemCount; }

    void    scrollLast(void);

	int		getCurIndex(void)		{ if(tSelected) return tSelected->iIndex; else return -1; }
	std::string getCurSIndex(void)		{ if(tSelected) return tSelected->sIndex; else return ""; }
	lv_subitem_t	*getCurSubitem(int index);


	lv_subitem_t	*getCurSub(void);

	int			getItemCount(void)		{ return iItemCount; }
	lv_item_t	*getItems(void)			{ return tItems; }
	lv_item_t	*getLastItem(void)	{ return tLastItem; }
	lv_item_t* getItem(int index);
	lv_item_t* getItem(const std::string& name);
	lv_subitem_t *getSubItem(int item_index, int subitem_index);
	lv_subitem_t *getSubItem(lv_item_t *it, int subitem_index);

	int		getClickedSub(void)		{ return iClickedSub; }
	gui_event_t *getWidgetEvent()	{ return &tLastWidgetEvent; }

    void    setShowSelect(bool s)   { bShowSelect = s; }
	void	setDrawBorder(bool _d)	{ bDrawBorder = _d; }

	void	SaveScrollbarPos(void)    { iSavedScrollbarPos = cScrollbar.getValue(); }
	void	RestoreScrollbarPos(void) { cScrollbar.setValue(iSavedScrollbarPos); iSavedScrollbarPos = 0; }

	inline bool	NeedsRepaint()  {return bNeedsRepaint; }
	inline void	SetRepaint(bool _r)  { bNeedsRepaint = _r; }  // Explicitly set this listview needs to be repainted

	// Read-only listview for skinning (typically text list), more variants to come.
	static CWidget * WidgetCreator( const std::vector< ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy )
	{
		CListview * w = new CListview();
		layout->Add( w, id, x, y, dx, dy );
		w->setOldStyle( p[0].b );
		w->setShowSelect( ! p[1].b );
		w->setDrawBorder( ! p[2].b );
		return w;
	};

	void	ProcessGuiSkinEvent(int iEvent)
	{
	};
};

}; // namespace DeprecatedGUI

#endif  //  __CLISTVIEW_H__DEPRECATED_GUI__
