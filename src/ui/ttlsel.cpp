#include <string>
#include <vector>

#include "data.h"
#include "ui.h"
#include "uiupdate.h"
#include "file.h"
#include "util.h"

//Where to start in titles
static int start = 0;

//Color shift for rect
static uint8_t clrShft = 0;
//Whether or not we're adding or subtracting from clrShft
static bool clrAdd = true;

//Selected rectangle X and Y.
static unsigned selRectX = 66, selRectY = 94;

static inline void reset()
{
    start = 0;
    selRectX = 66;
    selRectY = 94;
    data::selData = 0;
}

void ui::updateTitleMenu(const uint64_t& down, const uint64_t& held)
{
    if(clrAdd)
    {
        clrShft += 6;
        if(clrShft >= 0x72)
            clrAdd = false;
    }
    else
    {
        clrShft -= 3;
        if(clrShft <= 0)
            clrAdd = true;
    }

    unsigned x = 70, y = 98;

    unsigned endTitle = start + 32;
    if(start + 32 > (int)data::curUser.titles.size())
        endTitle = data::curUser.titles.size();

    //draw Rect so it's always behind icons
    drawBoundBox(selRectX, selRectY, 140, 140, clrShft);

    for(unsigned i = start; i < endTitle; y += 136)
    {
        unsigned endRow = i + 8;
        for(unsigned tX = x; i < endRow; i++, tX += 144)
        {
            if(i == endTitle)
                break;

            if((int)i == data::selData)
            {
                selRectX = tX - 6;
                selRectY = y - 6;

                unsigned titleWidth = textGetWidth(data::curData.getTitle().c_str(), ui::shared, 18);
                int rectWidth = titleWidth + 32, rectX = (tX + 64) - (rectWidth / 2);
                if(rectX < 16)
                    rectX = 16;
                else if(rectX + rectWidth > 1264)
                    rectX = 1264 - rectWidth;

                drawTextbox(frameBuffer, rectX, y - 50, rectWidth, 38);
                drawText(data::curData.getTitle().c_str(), frameBuffer, ui::shared, rectX + 16, y - 40, 18, ui::txtDiag);
            }
            if(data::curUser.titles[i].getFav())
                texDrawSkipNoAlpha(data::curUser.titles[i].getIconFav(), frameBuffer, tX, y);
            else
                texDrawSkipNoAlpha(data::curUser.titles[i].getIcon(), frameBuffer, tX, y);
        }
    }

    if(down & KEY_RIGHT)
    {
        if(data::selData < (int)data::curUser.titles.size() - 1)
            data::selData++;

        if(data::selData >= (int)start + 32)
            start += 8;
    }
    else if(down & KEY_LEFT)
    {
        if(data::selData > 0)
            data::selData--;

        if(data::selData < (int)start)
            start -= 8;
    }
    else if(down & KEY_UP)
    {
        data::selData -= 8;
        if(data::selData < 0)
            data::selData = 0;

        if(data::selData < start)
            start -= 8;
    }
    else if(down & KEY_DOWN)
    {
        data::selData += 8;
        if(data::selData > (int)data::curUser.titles.size() - 1)
            data::selData = data::curUser.titles.size() - 1;

        if(data::selData - start >= 32)
            start += 8;
    }
    else if(down & KEY_A && fs::mountSave(data::curUser, data::curData))
    {
        folderMenuPrepare(data::curUser, data::curData);
        //Wanna change this sometime
        ui::folderMenuInfo = util::getInfoString(data::curUser, data::curData);

        ui::changeState(FLD_SEL);
    }
    else if(down & KEY_Y)
        fs::dumpAllUserSaves(data::curUser);
    else if(down & KEY_MINUS)
    {
        if(ui::confirm(false, ui::confBlacklist.c_str(), data::curUser.titles[data::selData].getTitle().c_str()))
            data::blacklistAdd(data::curData);
    }
    else if(down & KEY_B)
    {
        reset();
        ui::changeState(USR_SEL);
        return;
    }
    else if(down & KEY_L)
    {
        if(--data::selUser < 0)
            data::selUser = data::users.size() - 1;

        reset();
        ui::showPopup(POP_FRAME_DEFAULT, data::curUser.getUsername().c_str());
    }
    else if(down & KEY_R)
    {
        if(++data::selUser > (int)data::users.size() - 1)
            data::selUser = 0;

        reset();
        ui::showPopup(POP_FRAME_DEFAULT, data::curUser.getUsername().c_str());
    }
    else if(down & KEY_ZR)
    {
        if(data::curData.getType() == FsSaveDataType_System)
            ui::showMessage("*NO*", "Deleting system save archives is disabled.");
        else if(confirm(true, ui::confEraseNand.c_str(), data::curData.getTitle().c_str()))
        {
            fsDeleteSaveDataFileSystemBySaveDataSpaceId(FsSaveDataSpaceId_User, data::curData.getSaveID());
            data::loadUsersTitles(false);
            data::selData = 0;
        }
    }
    else if(down & KEY_X)
        data::favoriteTitle(data::curData);
}

