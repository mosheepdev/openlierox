/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Local menu
// Created 30/6/02
// Jason Boettcher

#include <assert.h>

#include "LieroX.h"
#include "AuxLib.h"
#include "Graphics.h"
#include "CClient.h"
#include "Menu.h"
#include "CListview.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"


CGuiLayout cLocalMenu;

// Gui layout id's
enum {
	ml_Back=0,
	ml_Start,
	ml_Playing,
	ml_PlayerList,
	ml_LevelList,
	ml_Gametype,
	ml_ModName,
	ml_GameSettings,
    ml_WeaponOptions

	//ml_LoadingTimes,
	//ml_LoadingTimeLabel,
	//ml_Lives,
	//ml_MaxKills,
	//ml_TimeLimit,
	//ml_TagLimitLbl,
	//ml_TagLimitTxt,
};

int iGameType = GMT_DEATHMATCH;

bool	bGameSettings = false;
bool    bWeaponRest = false;

local_ply_t sLocalPlayers[MAX_PLAYERS];


///////////////////
// Initialize the local menu
void Menu_LocalInitialize(void)
{
	tMenu->iMenuType = MNU_LOCAL;
	iGameType = GMT_DEATHMATCH;
	bGameSettings = false;

	// Create the buffer
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_common,0,0);
	Menu_DrawSubTitleAdv(tMenu->bmpBuffer,SUB_LOCAL,15);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer, 15,100, 625, 465);

    // Minimap box
	Menu_DrawBox(tMenu->bmpBuffer, 133,129, 266, 230);

	Menu_RedrawMouse(true);

	// Shutdown any previous data from before
	cLocalMenu.Shutdown();
	cLocalMenu.Initialize();

	cLocalMenu.Add( new CButton(BUT_BACK, tMenu->bmpButtons), ml_Back, 25,440, 50,15);
	cLocalMenu.Add( new CButton(BUT_START, tMenu->bmpButtons), ml_Start, 555,440, 60,15);
	cLocalMenu.Add( new CListview(), ml_PlayerList,  410,115, 200, 120);
	cLocalMenu.Add( new CListview(), ml_Playing,     310,250, 300, 185);

	cLocalMenu.Add( new CButton(BUT_GAMESETTINGS, tMenu->bmpButtons), ml_GameSettings, 30, 325, 170,15);
    cLocalMenu.Add( new CButton(BUT_WEAPONOPTIONS,tMenu->bmpButtons), ml_WeaponOptions,30, 350, 185,15);

	cLocalMenu.Add( new CLabel("Mod",tLX->clNormalLabel),	    -1,         30,  284, 0,   0);
	cLocalMenu.Add( new CCombobox(),				ml_ModName,    120, 283, 170, 17);
	cLocalMenu.Add( new CLabel("Game type",tLX->clNormalLabel),	-1,         30,  260, 0,   0);
	cLocalMenu.Add( new CCombobox(),				ml_Gametype,   120, 259, 170, 17);
    cLocalMenu.Add( new CLabel("Level",tLX->clNormalLabel),	    -1,         30,  236, 0,   0);
	cLocalMenu.Add( new CCombobox(),				ml_LevelList,  120, 235, 170, 17);

	cLocalMenu.SendMessage(ml_Playing,		LVS_ADDCOLUMN, "Playing", 22);
	cLocalMenu.SendMessage(ml_Playing,		LVS_ADDCOLUMN, "", 300 - gfxGame.bmpTeamColours[0]->w - 50);
	cLocalMenu.SendMessage(ml_Playing,		LVS_ADDCOLUMN, "", -1);

	cLocalMenu.SendMessage(ml_Playing,		LVM_SETOLDSTYLE, (DWORD)1, 0);

	cLocalMenu.SendMessage(ml_PlayerList,	LVS_ADDCOLUMN, "Players", 22);
	cLocalMenu.SendMessage(ml_PlayerList,	LVS_ADDCOLUMN, "", 60);

	cLocalMenu.SendMessage(ml_PlayerList,		LVM_SETOLDSTYLE, (DWORD)0, 0);
	Menu_LocalAddProfiles();

	cLocalMenu.SendMessage(ml_Gametype,    CBS_ADDITEM, "Deathmatch", GMT_DEATHMATCH);
	cLocalMenu.SendMessage(ml_Gametype,    CBS_ADDITEM, "Team Deathmatch", GMT_TEAMDEATH);
	cLocalMenu.SendMessage(ml_Gametype,    CBS_ADDITEM, "Tag", GMT_TAG);
    cLocalMenu.SendMessage(ml_Gametype,    CBS_ADDITEM, "Demolitions", GMT_DEMOLITION);

	/*cLocalMenu.SendMessage(ml_Gametype,    CBS_ADDITEM,  "Capture the flag",1);
	cLocalMenu.SendMessage(ml_Gametype,    CBS_ADDITEM,   "Flag hunt",1);*/

    cLocalMenu.SendMessage(ml_Gametype,    CBM_SETCURSEL, tLXOptions->tGameinfo.nGameType, 0);
    iGameType = tLXOptions->tGameinfo.nGameType;

	// Fill the level list
	Menu_FillLevelList( (CCombobox *)cLocalMenu.getWidget(ml_LevelList), true);
	Menu_LocalShowMinimap(true);

	// Fill in the mod list
	Menu_Local_FillModList( (CCombobox *)cLocalMenu.getWidget(ml_ModName));

	// Fill in some game details
	tGameInfo.iLoadingTimes = tLXOptions->tGameinfo.iLoadingTime;
	tGameInfo.iLives = tLXOptions->tGameinfo.iLives;
	tGameInfo.iKillLimit = tLXOptions->tGameinfo.iKillLimit;
	tGameInfo.iBonusesOn = tLXOptions->tGameinfo.iBonusesOn;
	tGameInfo.iShowBonusName = tLXOptions->tGameinfo.iShowBonusName;

    // Initialize the local players
    for(int i=0; i<MAX_PLAYERS; i++)
        sLocalPlayers[i].bUsed = false;
}

//////////////
// Shutdown
void Menu_LocalShutdown(void)
{
	if (bGameSettings)
		Menu_GameSettingsShutdown();
	if (bWeaponRest)
		Menu_WeaponsRestrictionsShutdown();


	// Save the level and mod
	if (tLXOptions)  {
		cLocalMenu.SendMessage(ml_LevelList,CBS_GETCURSINDEX, &tLXOptions->tGameinfo.sMapFilename, 0);
		cLocalMenu.SendMessage(ml_ModName,CBS_GETCURSINDEX, &tLXOptions->tGameinfo.szModName, 0);
	}

	cLocalMenu.Shutdown();
}


///////////////////
// Local frame
void Menu_LocalFrame(void)
{
	gui_event_t *ev = NULL;
	mouse_t *Mouse = GetMouse();
	CListview *lv;
	profile_t *ply = NULL;

    // Game Settings
	if(bGameSettings) {
		if(Menu_GameSettings_Frame()) {
			// Re-do the buffer
			DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_common,0,0);
	        Menu_DrawSubTitleAdv(tMenu->bmpBuffer,SUB_LOCAL,15);
			if (tMenu->tFrontendInfo.bPageBoxes)
				Menu_DrawBox(tMenu->bmpBuffer, 15,100, 625, 465);
	        Menu_DrawBox(tMenu->bmpBuffer, 133,129, 266, 230);

			Menu_RedrawMouse(true);
			Menu_LocalShowMinimap(false);

			bGameSettings = false;
		}
		return;
	}


    // Weapons Restrictions
    if(bWeaponRest) {
		if(Menu_WeaponsRestrictions_Frame()) {
			// Re-do the buffer
			DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_common,0,0);
	        Menu_DrawSubTitleAdv(tMenu->bmpBuffer,SUB_LOCAL,15);
			if (tMenu->tFrontendInfo.bPageBoxes)
				Menu_DrawBox(tMenu->bmpBuffer, 15,100, 625, 465);
	        Menu_DrawBox(tMenu->bmpBuffer, 133,129, 266, 230);

			Menu_RedrawMouse(true);
			Menu_LocalShowMinimap(false);

			bWeaponRest = false;
		}
		return;
	}

	// Reload the list if user switches back to the game
	// Do not reload when a dialog is open
	if (bActivated)  {
		// Get the mod name
		cb_item_t *it = (cb_item_t *)cLocalMenu.SendMessage(ml_ModName,CBM_GETCURITEM,(DWORD)0,0);
		if(it)
			tLXOptions->tGameinfo.szModName = it->sIndex;

		// Fill in the mod list
		Menu_Local_FillModList( (CCombobox *)cLocalMenu.getWidget(ml_ModName));

		// Fill in the levels list
		cLocalMenu.SendMessage(ml_LevelList,CBS_GETCURSINDEX, &tLXOptions->tGameinfo.sMapFilename, 0);
		Menu_FillLevelList( (CCombobox *)cLocalMenu.getWidget(ml_LevelList), true);

		// Reload the minimap
		if (tLXOptions->tGameinfo.sMapFilename != "_random_")
			Menu_LocalShowMinimap(true);
	}


    // Was the mouse clicked on the map section
    if( Mouse->Up & SDL_BUTTON(1) ) {
        if( MouseInRect(136,132,128,96) )
            Menu_LocalShowMinimap(true);
    }


#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
		ev = cLocalMenu.Process();
	cLocalMenu.Draw(tMenu->bmpScreen);

	if(ev) {

		switch(ev->iControlID) {
			// Back
			case ml_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Shutdown
					Menu_LocalShutdown();

					// Leave
					PlaySoundSample(sfxGeneral.smpClick);
					Menu_MainInitialize();
					return;
				}
				break;

			// Start
			case ml_Start:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					PlaySoundSample(sfxGeneral.smpClick);

					// Start the game
					Menu_LocalStartGame();
				}
				break;

			// Player list
			case ml_PlayerList:
				if(ev->iEventMsg == LV_DOUBLECLK || ev->iEventMsg == LV_RIGHTCLK) {
					// Add the item to the players list
					lv = (CListview *)cLocalMenu.getWidget(ml_PlayerList);
					int index = lv->getCurIndex();

					// Check if we have enough room for another player
					if(Menu_LocalCheckPlaying(index)) {

						// Remove the item from the list
						lv->RemoveItem(index);

						ply = FindProfile(index);

						if(ply) {
                            // Add a player onto the local players list
							sLocalPlayers[index].bUsed = true;
							sLocalPlayers[index].nHealth = 0;
							sLocalPlayers[index].nTeam = 0;
							sLocalPlayers[index].psProfile = ply;

							// Add the item
							CImage *img = new CImage(gfxGame.bmpTeamColours[0]);
							if (img)  {
								img->setID(index);
								img->setRedrawMenu(false);
							}
							lv = (CListview *)cLocalMenu.getWidget(ml_Playing);
							lv->AddItem("",index,tLX->clListView);
							lv->AddSubitem(LVS_IMAGE, "", ply->bmpWorm, NULL);
							lv->AddSubitem(LVS_TEXT, ply->sName, NULL, NULL);
							lv->AddSubitem(LVS_WIDGET, "", NULL, img);

							// If we're in deathmatch, make the team colour invisible
							lv_subitem_t *sub = lv->getSubItem(lv->getLastItem(), 2);
							if(sub) {
								if(iGameType != GMT_TEAMDEATH)
									sub->iVisible = false;
								sub->iExtra = 0;
							}
						}
					}
				}
				break;

			// Playing list
			case ml_Playing:
				if(ev->iEventMsg == LV_DOUBLECLK || ev->iEventMsg == LV_RIGHTCLK) {

					// Put the player back into the players list
					lv = (CListview *)cLocalMenu.getWidget(ml_Playing);
					int index = lv->getCurIndex();

					// Remove the item from the list
					lv->RemoveItem(index);
					sLocalPlayers[index].bUsed = false;

					ply = FindProfile(index);

					// Add the player into the players list
					if(ply) {
						lv = (CListview *)cLocalMenu.getWidget(ml_PlayerList);
						lv->AddItem("", index, tLX->clListView);
						lv->AddSubitem(LVS_IMAGE, "", ply->bmpWorm, NULL);
						lv->AddSubitem(LVS_TEXT, ply->sName, NULL, NULL);
					}
				}


				if(ev->iEventMsg == LV_WIDGETEVENT && iGameType == GMT_TEAMDEATH) {

					// If the team colour item was clicked on, change it
					lv = (CListview *)cLocalMenu.getWidget(ml_Playing);
					
					ev = lv->getWidgetEvent();
					if (ev->cWidget->getType() == wid_Image && ev->iEventMsg == IMG_CLICK)  {
						lv_subitem_t *sub = lv->getSubItem(ev->iControlID, 2);

						if(sub) {
							sub->iExtra++;
							sub->iExtra %= 4;

							// Change the image
							((CImage *)ev->cWidget)->Change(gfxGame.bmpTeamColours[sub->iExtra]);
						}

						sLocalPlayers[ev->iControlID].nTeam = sub->iExtra;
						sLocalPlayers[ev->iControlID].psProfile->iTeam = sub->iExtra;
					}
				}
				break;


			// Game type
			case ml_Gametype:
				if(ev->iEventMsg == CMB_CHANGED) {
					iGameType = cLocalMenu.SendMessage(ml_Gametype, CBM_GETCURINDEX, (DWORD)0, 0);

					// Go through the items and enable/disable the team flags
					bool teams_on = iGameType == GMT_TEAMDEATH;
					CListview *lv = (CListview *)cLocalMenu.getWidget(ml_Playing);
					lv_item_t *it = lv->getItems();
					lv_subitem_t *sub = NULL;
					for (; it; it = it->tNext)  {
						sub = lv->getSubItem(it, 2);
						if (sub)
							sub->iVisible = teams_on;
					}
				}
				break;

			// Level list combo box
			case ml_LevelList:
				if(ev->iEventMsg == CMB_CHANGED) {

					Menu_LocalShowMinimap(true);
				}
				break;


			// Game settings button
			case ml_GameSettings:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					cLocalMenu.Draw( tMenu->bmpBuffer );

					bGameSettings = true;
					Menu_GameSettings();
				}
				break;


            // Weapons Restrictions button
            case ml_WeaponOptions:
                if( ev->iEventMsg == BTN_MOUSEUP ) {

					cLocalMenu.Draw( tMenu->bmpBuffer );

                    // Get the current mod
					cb_item_t *it = (cb_item_t *)cLocalMenu.SendMessage(ml_ModName,CBM_GETCURITEM,(DWORD)0,0);
                    if(it) {

					    bWeaponRest = true;
					    Menu_WeaponsRestrictions(it->sIndex);
                    }
                }
                break;

		}
	}

	// Draw the mouse
	DrawCursor(tMenu->bmpScreen);
}


///////////////////
// Add the profiles to the players list
void Menu_LocalAddProfiles(void)
{
	profile_t *p = GetProfiles();

	for(; p; p=p->tNext) {
		cLocalMenu.SendMessage( ml_PlayerList, LVS_ADDITEM, "", p->iID);
		cLocalMenu.SendMessage( ml_PlayerList, LVS_ADDSUBITEM, (DWORD)p->bmpWorm, LVS_IMAGE);
		cLocalMenu.SendMessage( ml_PlayerList, LVS_ADDSUBITEM, p->sName, LVS_TEXT);
	}
}


///////////////////
// Show the minimap
void Menu_LocalShowMinimap(bool bReload)
{
	// TODO: optimize or recode this!
	CMap map;
	static std::string buf;
	static std::string blah;

	cLocalMenu.SendMessage(ml_LevelList, CBS_GETCURSINDEX, &buf, 0);

    tGameInfo.sMapRandom.bUsed = false;

	// Draw a background over the old minimap
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack, 126,132,126,132,128,96);

	// Load the map
	blah = "levels/"+buf;
    if( bReload ) {

        // Create a random map
        if( buf == "_random_" ) {
            if( map.New(504,350,map.findRandomTheme()) ) {
			    map.ApplyRandom();

                // Free any old random map object list
                if( tGameInfo.sMapRandom.psObjects ) {
                    delete[] tGameInfo.sMapRandom.psObjects;
                    tGameInfo.sMapRandom.psObjects = NULL;
                }

                // Copy the layout
                maprandom_t *psRand = map.getRandomLayout();
                tGameInfo.sMapRandom = *psRand;
                tGameInfo.sMapRandom.bUsed = true;

                // Copy the objects, not link
                tGameInfo.sMapRandom.psObjects = new object_t[tGameInfo.sMapRandom.nNumObjects];
                if( tGameInfo.sMapRandom.psObjects ) {
                    for( int i=0; i<tGameInfo.sMapRandom.nNumObjects; i++ ) {
                        tGameInfo.sMapRandom.psObjects[i] = psRand->psObjects[i];
                    }
                }

                // Draw the minimap
		        DrawImage(tMenu->bmpMiniMapBuffer, map.GetMiniMap(), 0,0);
		        map.Shutdown();
            }

        } else {
            // Load the file
            if(map.Load(blah)) {

		        // Draw the minimap
		        DrawImage(tMenu->bmpMiniMapBuffer, map.GetMiniMap(), 0,0);

		        map.Shutdown();
	        }
        }
    }

	// Update the screen
    DrawImage(tMenu->bmpBuffer, tMenu->bmpMiniMapBuffer, 136,132);
	DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 130,130,130,130,140,110);
}


///////////////////
// Start a local game
void Menu_LocalStartGame(void)
{
    int i;

	// Level
	cLocalMenu.SendMessage(ml_LevelList, CBS_GETCURSINDEX, &tGameInfo.sMapname, 0);


	//
	// Players
	//
	CListview *lv_playing = (CListview *)cLocalMenu.getWidget(ml_Playing);

    // Calculate the number of players
    tGameInfo.iNumPlayers = lv_playing->getNumItems();

	// Can't start a game with no-one playing
	if(tGameInfo.iNumPlayers == 0)
		return;

    int count = 0;

    // Add the human players onto the list
    for(i=0; i<MAX_PLAYERS; i++) {
		if(sLocalPlayers[i].bUsed && sLocalPlayers[i].psProfile && sLocalPlayers[i].psProfile->iType == PRF_HUMAN) {
            tGameInfo.cPlayers[count++] = sLocalPlayers[i].psProfile;
            sLocalPlayers[i].psProfile->iTeam = sLocalPlayers[i].nTeam;
        }
    }

    // Add the unhuman players onto the list
    for(i=0; i<MAX_PLAYERS; i++) {
		if(sLocalPlayers[i].bUsed && sLocalPlayers[i].psProfile && sLocalPlayers[i].psProfile->iType != PRF_HUMAN) {
            tGameInfo.cPlayers[count++] = sLocalPlayers[i].psProfile;
            sLocalPlayers[i].psProfile->iTeam = sLocalPlayers[i].nTeam;
        }
    }

	// Save the current level in the options
	cLocalMenu.SendMessage(ml_LevelList, CBS_GETCURSINDEX, &tLXOptions->tGameinfo.sMapFilename, 0);

	//
	// Game Info
	//
	tGameInfo.iGameMode = cLocalMenu.SendMessage(ml_Gametype, CBM_GETCURINDEX, (DWORD)0, 0);
    tLXOptions->tGameinfo.nGameType = tGameInfo.iGameMode;

    tGameInfo.sPassword = "";


    // Get the mod name
	cb_item_t *it = (cb_item_t *)cLocalMenu.SendMessage(ml_ModName,CBM_GETCURITEM,(DWORD)0,0);
    if(it) {
        tGameInfo.sModName = it->sIndex;
        tLXOptions->tGameinfo.szModName = it->sIndex;
    } else {

		// Couldn't find a mod to load
		cLocalMenu.Draw(tMenu->bmpBuffer);
		Menu_MessageBox("Error","Could not find a mod to load!", LMB_OK);
		DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_common,0,0);
        Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
		Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_LOCAL);
		Menu_RedrawMouse(true);
		return;
	}

	*bGame = true;
	tMenu->iMenuRunning = false;
	tGameInfo.iGameType = GME_LOCAL;

	cLocalMenu.Shutdown();
}


///////////////////
// Check if we can add another player to the list
int Menu_LocalCheckPlaying(int index)
{
	uint		plycount = 0;
	uint		hmncount = 0;
	uint		i, count;
	profile_t	*p;

    count = 0;
    for(i=0; i<MAX_PLAYERS; i++) {
        if(sLocalPlayers[i].bUsed)
            count++;
    }

	// Go through the playing list
	for(i=0; i<MAX_PLAYERS; i++) {
        if(!sLocalPlayers[i].bUsed)
            continue;

		if(sLocalPlayers[i].psProfile->iType == PRF_HUMAN)
			hmncount++;
		plycount++;
	}

	p = FindProfile(index);

	// TODO: does it work with this removed restrictions ?
	//	if not, make it working
	// Doesn't work and cannot work for now, better uncomment the
	// restrictions for now (it could lead to a crash)

	// Check if there is too many players
	if(plycount >= MAX_PLAYERS)
		return false;

	// Check if there is too many human players (MAX: 2)
	if(p) {
		if(p->iType == PRF_HUMAN && hmncount+1 > 2)
			return false;
	}

	return true;
}


	class addMod { public:
		CCombobox* combobox;
		int* baseid;
		int i;
		addMod(CCombobox* cb_, int* id_) : combobox(cb_), baseid(id_), i(0) {}
		inline bool operator() (const std::string& f) {
			size_t sep = findLastPathSep(f);
			if(sep != std::string::npos) {
				std::string name;
				if(CGameScript::CheckFile(f,name)
				&& !combobox->getItem(name)) {
					combobox->addItem(i,f.substr(sep+1),name);

					// Store the index of the last used mod
					if(stringcasecmp(f.substr(sep+1),tLXOptions->tGameinfo.szModName) == 0)
						*baseid = i;
					i++;
				}
			}

			return true;
		}
	};


///////////////////
// Fill in the mod list
void Menu_Local_FillModList( CCombobox *cb )
{
	// Find all directories in the the lierox
	int baseid = 0;

	cb->clear();

	FindFiles(addMod(cb,&baseid),".",FM_DIR);

	// Set the last used mod as default
	if(baseid >= 0)
		cb->setCurItem(baseid);

	// Sort the mod list ascending
	cb->Sort(true);
}




/*
=======================

	 Game Settings

 For both local & host

=======================
*/


CGuiLayout		cGameSettings;

// Game Settings
enum {
	gs_Ok,
    gs_Default,
	gs_Lives,
	gs_MaxKills,
	gs_LoadingTime,
	gs_LoadingTimeLabel,
	gs_Bonuses,
	gs_ShowBonusNames,
	gs_MaxTime,
	gs_Tournament
};



///////////////////
// Initialize the game settings
void Menu_GameSettings(void)
{
	// Setup the buffer
	Menu_DrawBox(tMenu->bmpBuffer, 120,150, 520,440);
	DrawRectFillA(tMenu->bmpBuffer, 122,152, 518,438, tLX->clDialogBackground, 200);

	// Lives
	// Max kills
	// Weapon Loading time
	// Bonuses
	// Show bonus names
	// Max Time
	//


	Menu_RedrawMouse(true);

	cGameSettings.Initialize();
	cGameSettings.Add( new CLabel("Game Settings", tLX->clNormalLabel),		    -1,	        270,155, 0, 0);
	cGameSettings.Add( new CButton(BUT_OK, tMenu->bmpButtons),	    gs_Ok,      220,420, 40,15);
    cGameSettings.Add( new CButton(BUT_DEFAULT, tMenu->bmpButtons), gs_Default, 350,420, 80,15);
	cGameSettings.Add( new CLabel("Lives", tLX->clNormalLabel),				    -1,	        150,200, 0, 0);
	cGameSettings.Add( new CLabel("Max Kills", tLX->clNormalLabel),			    -1,	        150,230, 0, 0);
	cGameSettings.Add( new CLabel("Loading Time", tLX->clNormalLabel),		    -1,	        150,260, 0, 0);
	cGameSettings.Add( new CLabel("Bonuses", tLX->clNormalLabel),			    -1,	        150,290, 0, 0);
	cGameSettings.Add( new CLabel("Show Bonus names", tLX->clNormalLabel),	    -1,	        150,320, 0, 0);
	if (tGameInfo.iGameType == GME_HOST)
		cGameSettings.Add( new CLabel("Tournament mode", tLX->clNormalLabel),	    -1,	        150,350, 0, 0);
	//cGameSettings.Add( new CLabel("Max Kills", tLX->clNormalLabel),			-1,	   150,240, 0, 0);

	cGameSettings.Add( new CTextbox(),							gs_Lives,		320,197, 100,tLX->cFont.GetHeight());
	cGameSettings.Add( new CTextbox(),							gs_MaxKills,	320,227, 100,tLX->cFont.GetHeight());
	cGameSettings.Add( new CSlider(500),						gs_LoadingTime,315,257, 160,20);
	cGameSettings.Add( new CLabel("", tLX->clNormalLabel),					gs_LoadingTimeLabel, 480, 260, 0, 0);
	cGameSettings.Add( new CCheckbox(tLXOptions->tGameinfo.iBonusesOn),	gs_Bonuses, 320,287,17,17);
	cGameSettings.Add( new CCheckbox(tLXOptions->tGameinfo.iShowBonusName),gs_ShowBonusNames, 320,317,17,17);
	cGameSettings.Add( new CCheckbox(tLXOptions->tGameinfo.bTournament),gs_Tournament, 320,347,17,17);

	if (tGameInfo.iGameType != GME_HOST)
		cGameSettings.getWidget(gs_Tournament)->setEnabled(false);

	cGameSettings.SendMessage(gs_Lives,TXM_SETMAX,6,0);
	cGameSettings.SendMessage(gs_MaxKills,TXM_SETMAX,6,0);

	cGameSettings.SendMessage(gs_LoadingTime, SLM_SETVALUE, tLXOptions->tGameinfo.iLoadingTime, 0);

	if(tLXOptions->tGameinfo.iLives >= 0)
		cGameSettings.SendMessage(gs_Lives, TXS_SETTEXT, itoa(tLXOptions->tGameinfo.iLives), 0);
	if(tLXOptions->tGameinfo.iKillLimit >= 0)
		cGameSettings.SendMessage(gs_MaxKills, TXS_SETTEXT, itoa(tLXOptions->tGameinfo.iKillLimit), 0);
	//if(tLXOptions->tGameinfo.iTimeLimit >= 0)
	//	cLocalMenu.SendMessage(gs_TimeLimit, TXS_SETTEXT, itoa(tLXOptions->tGameinfo.iTimeLimit), 0);
	//if(tLXOptions->tGameinfo.iTagLimit >= 0)
		//cLocalMenu.SendMessage(gs_TagLimitTxt, TXS_SETTEXT, itoa(tLXOptions->tGameinfo.iTagLimit), 0);
}

/////////////
// Shutdown
void Menu_GameSettingsShutdown(void)
{
	cGameSettings.Shutdown();
}


///////////////////
// Game settings frame
// Returns whether of not we have finised with the game settings
bool Menu_GameSettings_Frame(void)
{
	gui_event_t *ev = NULL;

	DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 120,150, 120,150, 400,300);

#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
		ev = cGameSettings.Process();
	cGameSettings.Draw(tMenu->bmpScreen);

	if(ev) {

		switch(ev->iControlID) {

			// OK, done
			case gs_Ok:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					Menu_GameSettings_GrabInfo();
					Menu_GameSettingsShutdown();

					return true;
				}
				break;

            // Set the default values
            case gs_Default:
                if( ev->iEventMsg == BTN_MOUSEUP ) {
                    Menu_GameSettings_Default();
                }
                break;
		}
	}

	// Set the value of the loading time label
	int l = cGameSettings.SendMessage(gs_LoadingTime, SLM_GETVALUE, 100, 0);
	static std::string lstr; lstr = itoa(l)+"%";
	cGameSettings.SendMessage(gs_LoadingTimeLabel, LBS_SETTEXT, lstr, 0);

	// Draw the mouse
	DrawCursor(tMenu->bmpScreen);

	return false;
}


///////////////////
// Grab the game settings info
void Menu_GameSettings_GrabInfo(void)
{
	static std::string buf;

	tLXOptions->tGameinfo.iLoadingTime = cGameSettings.SendMessage(gs_LoadingTime, SLM_GETVALUE, 100, 0);
	tGameInfo.iLoadingTimes = tLXOptions->tGameinfo.iLoadingTime;

	// Default to no setting
	tGameInfo.iLives = tLXOptions->tGameinfo.iLives = -2;
	tGameInfo.iKillLimit = tLXOptions->tGameinfo.iKillLimit = -1;
	tGameInfo.iTimeLimit = tLXOptions->tGameinfo.iTimeLimit = -1;
	tGameInfo.iTagLimit = tLXOptions->tGameinfo.iTagLimit = -1;
	tGameInfo.iBonusesOn = true;
	tGameInfo.iShowBonusName = true;
	tGameInfo.bTournament = false;


	// Store the game info into the options structure as well
	cGameSettings.SendMessage(gs_Lives, TXS_GETTEXT, &buf, 0);
	if(buf != "") {
		tLXOptions->tGameinfo.iLives = atoi(buf);
		tGameInfo.iLives = atoi(buf);
	}
	cGameSettings.SendMessage(gs_MaxKills, TXS_GETTEXT, &buf, 0);
	if(buf != "") {
		tLXOptions->tGameinfo.iKillLimit = atoi(buf);
		tGameInfo.iKillLimit = atoi(buf);
	}

	tGameInfo.iBonusesOn = cGameSettings.SendMessage( gs_Bonuses, CKM_GETCHECK, (DWORD)0, 0);
	tLXOptions->tGameinfo.iBonusesOn = tGameInfo.iBonusesOn;

	tGameInfo.iShowBonusName = cGameSettings.SendMessage( gs_ShowBonusNames, CKM_GETCHECK, (DWORD)0, 0);
	tLXOptions->tGameinfo.iShowBonusName = tGameInfo.iShowBonusName;

	tGameInfo.bTournament = cGameSettings.SendMessage( gs_Tournament, CKM_GETCHECK, (DWORD)0, 0) != 0;
	tLXOptions->tGameinfo.bTournament = tGameInfo.bTournament;
}


///////////////////
// Set the default game settings info
void Menu_GameSettings_Default(void)
{
    cGameSettings.SendMessage(gs_LoadingTime, SLM_SETVALUE, 100, 0);

    cGameSettings.SendMessage(gs_Lives, TXS_SETTEXT, "10", 0);
	cGameSettings.SendMessage(gs_MaxKills, TXS_SETTEXT, "", 0);

    cGameSettings.SendMessage(gs_Bonuses, CKM_SETCHECK, (DWORD)1, 0);
    cGameSettings.SendMessage(gs_ShowBonusNames, CKM_SETCHECK, (DWORD)1, 0);
	cGameSettings.SendMessage(gs_Tournament, CKM_SETCHECK, (DWORD)0, 0);
}




/*
=======================

  Weapons Restrictions

 For both local & host

=======================
*/


CGuiLayout		cWeaponsRest;
CWpnRest        cWpnRestList;
CGameScript     *cWpnGameScript = NULL;

// Weapons Restrictions
enum {
	wr_Ok,
    wr_Scroll,
    wr_Reset,
	wr_ListBox,
	wr_Cancel,
    wr_Random,
	wr_Load,
	wr_Save
};



///////////////////
// Initialize the weapons restrictions
void Menu_WeaponsRestrictions(const std::string& szMod)
{

	// Setup the buffer
	DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_common, 120,150,120,150, 400,330);
	Menu_DrawBox(tMenu->bmpBuffer, 120,150, 520,470);
	//DrawRectFillA(tMenu->bmpBuffer, 122,152, 518,438, 0, 100);


	Menu_RedrawMouse(true);

	cWeaponsRest.Initialize();
	cWeaponsRest.Add( new CLabel("Weapon Options", tLX->clNormalLabel),     -1,        275,155, 0, 0);
    cWeaponsRest.Add( new CButton(BUT_RESET, tMenu->bmpButtons),wr_Reset,  180,420, 60,15);
    cWeaponsRest.Add( new CButton(BUT_RANDOM, tMenu->bmpButtons),wr_Random,280,420, 80,15);
    cWeaponsRest.Add( new CButton(BUT_OK, tMenu->bmpButtons),	wr_Ok,     400,420, 30,15);
    cWeaponsRest.Add( new CButton(BUT_LOAD, tMenu->bmpButtons),	wr_Load,   250,445, 60,15);
    cWeaponsRest.Add( new CButton(BUT_SAVE, tMenu->bmpButtons),	wr_Save,   330,445, 60,15);
    cWeaponsRest.Add( new CScrollbar(),                         wr_Scroll, 490,185, 14,230);

    // Load the weapons
    cWpnRestList.loadList("cfg/wpnrest.dat");


    //
    // Update the list with the currently selected mod
    //

    cWpnGameScript = new CGameScript;
    if( cWpnGameScript ) {
        if( cWpnGameScript->Load(szMod) )
            cWpnRestList.updateList( cWpnGameScript );
    }
}

//////////////////
// Shutdown the weapon restrictions
void Menu_WeaponsRestrictionsShutdown(void)
{
	cWeaponsRest.Shutdown();

    cWpnRestList.saveList("cfg/wpnrest.dat");
    cWpnRestList.Shutdown();

	if (cWpnGameScript)  {
		cWpnGameScript->Shutdown();
		delete cWpnGameScript;
		cWpnGameScript = NULL;
	}
}


///////////////////
// Weapons Restrictions frame
// Returns whether or not we have finished with the weapons restrictions
bool Menu_WeaponsRestrictions_Frame(void)
{
	gui_event_t *ev = NULL;
	mouse_t *Mouse = GetMouse();
    //Uint32 blue = MakeColour(0,138,251);

    assert(cWpnGameScript);

    // State strings
    static const std::string    szStates[] = {"Enabled", "Bonus", "Banned"};

	DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 120,150, 120,150, 400,300);

    // Draw the list
    wpnrest_t *psWpn = cWpnRestList.getList();
    int num = cWpnRestList.getNumWeapons();
    int count = cWeaponsRest.SendMessage(wr_Scroll, SCM_GETVALUE,(DWORD)0,0);
    int weaponCount = 0;
    int i, j = 0, w = 0;

    // Count the number of weapons in _this_ mod only
    for(i=0; i<num; i++) {
        if(cWpnGameScript->weaponExists(psWpn[i].psLink->szName))
            weaponCount++;
    }

    // Show the weapons
	static std::string buf;
    for(i=0; i<num; i++) {
        if(!cWpnGameScript->weaponExists(psWpn[i].psLink->szName))
            continue;
        if( w++ < count )
            continue;
        if( j > 10 )
            break;


        int y = 190 + (j++)*20;
        Uint32 Colour = tLX->clNormalLabel;

        // If a mouse is over the line, highlight it
        if( Mouse->X > 150 && Mouse->X < 450 ) {
            if( Mouse->Y > y && Mouse->Y < y+20 ) {
                Colour = tLX->clMouseOver;

                // If the mouse has been clicked, cycle through the states
                if( Mouse->Up & SDL_BUTTON(1) ) {
                    psWpn[i].psLink->nState++;
                    psWpn[i].psLink->nState %= 3;
                }
            }
        }

		buf = psWpn[i].psLink->szName; 
		stripdot(buf,245);
        tLX->cFont.Draw( tMenu->bmpScreen, 150, y, Colour, buf );
        tLX->cFont.Draw( tMenu->bmpScreen, 400, y, Colour, szStates[psWpn[i].psLink->nState] );
    }

    // Adjust the scrollbar
    cWeaponsRest.SendMessage(wr_Scroll, SCM_SETITEMSPERBOX, 12, 0);
    cWeaponsRest.SendMessage(wr_Scroll, SCM_SETMIN, (DWORD)0, 0);
    if(weaponCount>10)
        cWeaponsRest.SendMessage(wr_Scroll, SCM_SETMAX, weaponCount+1, 0);
    else
        cWeaponsRest.SendMessage(wr_Scroll, SCM_SETMAX, (DWORD)0, 0);


#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
		ev = cWeaponsRest.Process();
	cWeaponsRest.Draw(tMenu->bmpScreen);

	if(ev) {

		if(ev->iEventMsg == SDL_BUTTON_WHEELUP)  {
			CScrollbar *tScrollbar = (CScrollbar *)cWeaponsRest.getWidget(wr_Scroll);
			tScrollbar->MouseWheelUp(NULL);
		}

		if(ev->iEventMsg == SDL_BUTTON_WHEELDOWN)  {
			CScrollbar *tScrollbar = (CScrollbar *)cWeaponsRest.getWidget(wr_Scroll);
			tScrollbar->MouseWheelDown(NULL);
		}


		switch(ev->iControlID) {

			// OK, done
			case wr_Ok:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					Menu_WeaponsRestrictionsShutdown();

					return true;
				}
				break;

            // Reset the list
            case wr_Reset:
                if( ev->iEventMsg == BTN_MOUSEUP ) {
                    cWpnRestList.cycleVisible(cWpnGameScript);
                }
                break;

            // Randomize the list
            case wr_Random:
                if(ev->iEventMsg == BTN_MOUSEUP) {
                    cWpnRestList.randomizeVisible(cWpnGameScript);
                }
                break;

            // Open the load dialog
            case wr_Load:
                if(ev->iEventMsg == BTN_MOUSEUP) {
                    Menu_WeaponPresets(false,&cWpnRestList);
                }
                break;

            // Open the save dialog
            case wr_Save:
                if(ev->iEventMsg == BTN_MOUSEUP) {
                    Menu_WeaponPresets(true,&cWpnRestList);
                }
                break;
		}
	}

	// Draw the mouse
	DrawCursor(tMenu->bmpScreen);

	return false;
}


///////////////////
// Weapon presets load/save
// For both local and host
enum  {
	wp_Cancel=0,
	wp_Ok,
	wp_PresetList,
	wp_PresetName
};

CGuiLayout cWpnPresets;

	class addWeaponPresets { public:
		CListview* listview;
		addWeaponPresets(CListview* lv_) : listview(lv_) {}
		inline bool operator() (const std::string& f) {
			if(stringcasecmp(GetFileExtension(f),"wps")) {
				size_t sep = findLastPathSep(f);
				if(sep != std::string::npos) {
					std::string name = f.substr(sep+1);
					if(!listview->getItem(name)) {
						listview->AddItem(name,0,tLX->clListView);
						listview->AddSubitem(LVS_TEXT, name, NULL, NULL);
					}
				}
			}
			return true;
		}
	};

void Menu_WeaponPresets(bool save, CWpnRest *wpnrest)
{
	if (!wpnrest)
		return;

	keyboard_t *kb = GetKeyboard();
	gui_event_t *ev = NULL;
	int quitloop = false;
	CTextbox *t;

	// Save the background
	DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpScreen, 0,0, 0,0, 640,480);

	Menu_RedrawMouse(true);

	cWpnPresets.Initialize();

	cWpnPresets.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons), wp_Cancel, 180,310, 75,15);
	cWpnPresets.Add( new CButton(BUT_OK, tMenu->bmpButtons),     wp_Ok, 430,310, 40,15);
	cWpnPresets.Add( new CListview(),                            wp_PresetList, 180,170, 280,110+(!save)*20);
	cWpnPresets.Add( new CTextbox(),                             wp_PresetName, 270,285, 190,tLX->cFont.GetHeight());

	cWpnPresets.SendMessage(wp_PresetList,LVM_SETOLDSTYLE,(DWORD)0,0);

	t = (CTextbox *)cWpnPresets.getWidget(wp_PresetName);

	// Hide the textbox for Load
	t->setEnabled(save);

	// Load the level list

	CListview *lv = (CListview *)cWpnPresets.getWidget(wp_PresetList);
	lv->AddColumn("Weapon presets",60);

	FindFiles(addWeaponPresets(lv),"cfg/presets/",FM_REG);



	ProcessEvents();
	while(!kb->KeyUp[SDLK_ESCAPE] && !quitloop && tMenu->iMenuRunning) {
		Menu_RedrawMouse(false);
		ProcessEvents();

		//DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 170,150, 170,150, 300, 180);
		Menu_DrawBox(tMenu->bmpScreen, 170, 150, 470, 330);
		DrawImageAdv(tMenu->bmpScreen, tMenu->bmpMainBack_common, 172,152, 172,152, 297,177);
		DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_common, 172,152, 172,152, 297,177);

		tLX->cFont.DrawCentre(tMenu->bmpScreen, 320, 155, tLX->clNormalLabel, save ? "Save" : "Load");
		if (save)
			tLX->cFont.Draw(tMenu->bmpScreen, 180,288,tLX->clNormalLabel,"Preset name");

#ifdef WITH_MEDIAPLAYER
		if (!cMediaPlayer.GetDrawPlayer())
#endif
			ev = cWpnPresets.Process();
		cWpnPresets.Draw(tMenu->bmpScreen);

		// Process the widgets
#ifdef WITH_MEDIAPLAYER
		if(ev && !cMediaPlayer.GetDrawPlayer()) {
#else
		if(ev)  {
#endif

			switch(ev->iControlID) {
				// Cancel
				case wp_Cancel:
					if(ev->iEventMsg == BTN_MOUSEUP) {
						PlaySoundSample(sfxGeneral.smpClick);
						quitloop = true;
					}
					break;

				// Presets list
				case wp_PresetList:
					if(ev->iEventMsg != LV_NONE) {
						t->setText( lv->getCurSIndex() );
					}
				break;
			}

			// OK and double click on listview
			if (ev->iControlID == wp_Ok || ev->iControlID == wp_PresetList)  {
				if((ev->iEventMsg == BTN_MOUSEUP && ev->iControlID == 1) || ev->iEventMsg == LV_DOUBLECLK) {

					// Play the sound only for OK button
					if (ev->iControlID == wp_Ok)
						PlaySoundSample(sfxGeneral.smpClick);

					// Don't process when nothing is selected
					if(t->getText().length() > 0) {

						quitloop = true;
						static std::string buf;
						if(save) {

							// Save
							buf = std::string("cfg/presets/") + t->getText();

							// Check if it exists already. If so, ask user if they wanna overwrite
							if(Menu_WeaponPresetsOkSave(buf))
								wpnrest->saveList(buf);
							else
								quitloop = false;
						} else {

							// Load
							buf = std::string("cfg/presets/") + t->getText();
							wpnrest->loadList(buf);
						}
					}
				}
			}
		}

		// Draw mouse
		DrawCursor(tMenu->bmpScreen);

		// Display the dialog
		FlipScreen(tMenu->bmpScreen);
	}

	// Redraw back to normal
	DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_common, 120,150,122,152, 396,316);
	DrawImage(tMenu->bmpScreen,tMenu->bmpBuffer,0,0);

	Menu_RedrawMouse(true);

	Menu_WeaponPresetsShutdown();
}

/////////////
// Shutdown
void Menu_WeaponPresetsShutdown(void)
{
	cWpnPresets.Shutdown();
}


///////////////////
// Check if there is a possible overwrite
int Menu_WeaponPresetsOkSave(const std::string& szFilename)
{
	std::string filename = szFilename;

	// Adjust the filename
	if( stringcasecmp(GetFileExtension( szFilename ), "wps") != 0)
		filename += ".wps";

	FILE *fp = OpenGameFile(filename,"rb");
	if( fp == NULL)
		// File doesn't exist, ok to save
		return true;

	fclose(fp);


	// The file already exists, show a message box to confirm the overwrite
	int nResult = Menu_MessageBox("Confirmation","The preset already exists. Overwrite?", LMB_YESNO);
	if( nResult == MBR_YES )
		return true;


	// No overwrite
	return false;
}

