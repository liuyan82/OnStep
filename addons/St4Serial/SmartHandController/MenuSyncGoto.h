// SyncGoto menu

void SmartHandController::menuSyncGoto(bool sync)
{
  current_selection_L1 = 1;
  while (current_selection_L1 != 0) {
    const char *string_list_gotoL1 = "Star\nMessier\nCaldwell\nHerschel\nSolar System\nUser Catalog\nCoordinates\nHome";
    current_selection_L1 = display->UserInterfaceSelectionList(&buttonPad, sync ? "Sync" : "Goto", current_selection_L1, string_list_gotoL1);
    switch (current_selection_L1) {
      case 1:
        menuStar(sync);
        break;
      case 2:
        menuMessier(sync);
        break;
      case 3:
	      menuCaldwell(sync);
      break;
      case 4:
        menuHerschel(sync);
        break;
      case 5:
        menuSolarSys(sync);
        break;
      case 6:
        menuUser(sync);
        break;
      case 7:
        menuRADec(sync);
        break;
      case 8:
      {
        boolean GotoHome=false; 
        DisplayMessage("Goto Home will", "clear the Model", 2000);
        if (display->UserInterfaceInputValueBoolean(&buttonPad, "Goto Home?", &GotoHome)) 
             if (GotoHome)
             {
        char cmd[5];
        sprintf(cmd, ":hX#");
        cmd[2] = sync ? 'F' : 'C';
        if (SetLX200(cmd) == LX200VALUESET) DisplayMessage(sync ? "Reset at" : "Goto", " Home Position", -1);
        // Quit Menu
        current_selection_L1 = 0;
        current_selection_L0 = 0;
      }
      }
      break;
    }
  }
}

void SmartHandController::menuMessier(bool sync)
{
  if (!cat_mgr.isInitialized()) { cat_mgr.setLat(telInfo.getLat()); cat_mgr.setLstT0(telInfo.getLstT0()); }
  cat_mgr.select(MESSIER);
  cat_mgr.filter(FM_ABOVE_HORIZON);
  cat_mgr.setIndex(0);
  
  if (cat_mgr.isInitialized()) {
    if (display->UserInterfaceCatalog(&buttonPad, sync ? "Sync Messier" : "Goto Messier")) {
      bool ok = DisplayMessageLX200(SyncGotoCatLX200(sync), false);
      if (ok) { current_selection_L1 = 0; current_selection_L0 = 0; } // Quit Menu
    }
  }
}

void SmartHandController::menuCaldwell(bool sync)
{
  if (!cat_mgr.isInitialized()) { cat_mgr.setLat(telInfo.getLat()); cat_mgr.setLstT0(telInfo.getLstT0()); }
  cat_mgr.select(CALDWELL);
  cat_mgr.filter(FM_ABOVE_HORIZON);
  cat_mgr.setIndex(0);
  
  if (cat_mgr.isInitialized()) {
    if (display->UserInterfaceCatalog(&buttonPad, sync ? "Sync Caldwell" : "Goto Caldwell")) {
      bool ok = DisplayMessageLX200(SyncGotoCatLX200(sync), false);
      if (ok) { current_selection_L1 = 0; current_selection_L0 = 0; } // Quit Menu
    }
  }
}

void SmartHandController::menuStar(bool sync)
{
  if (!cat_mgr.isInitialized()) { cat_mgr.setLat(telInfo.getLat()); cat_mgr.setLstT0(telInfo.getLstT0()); }
  cat_mgr.select(STAR);
  cat_mgr.filter(FM_ABOVE_HORIZON);
  cat_mgr.setIndex(0);
  if (cat_mgr.isInitialized()) {
    if (display->UserInterfaceCatalog(&buttonPad, sync ? "Sync Star" : "Goto Star")) {
      bool  ok = DisplayMessageLX200(SyncGotoCatLX200(sync), false);
      if (ok) { current_selection_L1 = 0; current_selection_L0 = 0; } // Quit Menu
    }
  }
}

void SmartHandController::menuSolarSys(bool sync)
{
  if (current_selection_SolarSys<1) current_selection_SolarSys=1;

  const char *string_list_SolarSyst = "Sun\nMercury\nVenus\nMars\nJupiter\nSaturn\nUranus\nNeptune\nMoon";
  current_selection_SolarSys = display->UserInterfaceSelectionList(&buttonPad, sync ? "Sync" : "Goto", current_selection_SolarSys, string_list_SolarSyst);
  
  if (current_selection_SolarSys == 0) return;
  current_selection_SolarSys > 3 ? current_selection_SolarSys : current_selection_SolarSys--;
  
  if (current_selection_SolarSys == 0)
  { 
    boolean GotoSun=false; 
    DisplayMessage("Pointing at the Sun", "can be dangerous", 2000);
    if (display->UserInterfaceInputValueBoolean(&buttonPad, "Goto Sun?", &GotoSun))
      { if (!GotoSun) return;
      }
    else return;
  }
  
  bool ok = DisplayMessageLX200(SyncGotoPlanetLX200(sync, current_selection_SolarSys),false);
  if (current_selection_SolarSys != 0 && ok) { current_selection_L1 = 0; current_selection_L0 = 0; } // Quit Menu
}

void SmartHandController::menuHerschel(bool sync)
{
  if (!cat_mgr.isInitialized()) { cat_mgr.setLat(telInfo.getLat()); cat_mgr.setLstT0(telInfo.getLstT0()); }
  cat_mgr.select(HERSCHEL);
  cat_mgr.filter(FM_ABOVE_HORIZON);
  cat_mgr.setIndex(0);
  
  if (cat_mgr.isInitialized()) {
    if (display->UserInterfaceCatalog(&buttonPad, sync ? "Sync Herschel" : "Goto Herschel")) {
      bool ok = DisplayMessageLX200(SyncGotoCatLX200(sync), false);
      if (ok) { current_selection_L1 = 0; current_selection_L0 = 0; } // Quit Menu
    }
  }
}

void SmartHandController::menuUser(bool sync)
{
  char string_list_UserCatalogs[240] = "";

  // read user catalogs names into a list
  int userCatalog[15];
  int i=0;
  char temp[10];
  char temp1[500];
  for (int l=0; l<=14; l++) {
    strcpy(temp,":Lo0#"); temp[3]=l+'0';
    SetLX200(temp);
    SetLX200(":L$#");
    GetLX200(":LI#",temp1);
    
    int len = strlen(temp1);
    if (len>4) temp1[len-5]=0;
    if (temp1[0]==0) continue;

    if (l!=0) strcat(string_list_UserCatalogs,"\n");
    strcat(string_list_UserCatalogs,(char*)(&temp1[1]));
    userCatalog[i++]=l;
  }

  if (i>0) {
    if (current_selection_UserCatalog<1) current_selection_UserCatalog=1;
    while (current_selection_UserCatalog>0) {
      int last_selection_UserCatalog = current_selection_UserCatalog;
      current_selection_UserCatalog = display->UserInterfaceSelectionList(&buttonPad, "Select User Cat", current_selection_UserCatalog, string_list_UserCatalogs);

      if (current_selection_UserCatalog>0) {
        // select user catalog
        strcpy(temp,":Lo0#"); temp[3]=userCatalog[current_selection_UserCatalog-1]+'0';
        SetLX200(temp);
    
        // show the catalog objects
        if (display->UserInterfaceUserCatalog(&buttonPad, sync ? "Sync User Item" : "Goto User Item")) {
          bool ok = DisplayMessageLX200(SetLX200(":LIG#"));
          if (ok) { current_selection_L1 = 0; current_selection_L0 = 0; current_selection_UserCatalog = 0; } // Quit all menus, selection succeeded
        }
      }
    }
    
  } else  current_selection_L1 = 0; // Quit this menu, nothing to display
}

void SmartHandController::menuRADec(bool sync)
{
  if (display->UserInterfaceInputValueRA(&buttonPad, &angleRA))
  {
//    uint8_t vr1, vr2, vr3, vd2, vd3;
//    short vd1;
//    gethms(angleRA, vr1, vr2, vr3);
    float fR;
    secondsToFloat(angleRA,fR);
    if (display->UserInterfaceInputValueDec(&buttonPad, &angleDEC))
    {
      float fD;
      secondsToFloat(angleDEC,fD);
//      getdms(angleDEC, vd1, vd2, vd3);
//      bool ok = DisplayMessageLX200(SyncGotoLX200(sync, vr1, vr2, vr3, vd1, vd2, vd3));
      bool ok = DisplayMessageLX200(SyncGotoLX200(sync, fR, fD));
      if (ok)
      {
        // Quit Menu
        current_selection_L1 = 0;
        current_selection_L0 = 0;
      }
    }
  }
}
