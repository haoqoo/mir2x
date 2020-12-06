/*
 * =====================================================================================
 *
 *       Filename: fixedlocmagic.cpp
 *        Created: 08/07/2017 21:31:24
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include "sdldevice.hpp"
#include "dbcomrecord.hpp"
#include "pngtexoffdb.hpp"
#include "fixedlocmagic.hpp"

extern SDLDevice *g_SDLDevice;
extern PNGTexOffDB *g_magicDB;

void FixedLocMagic::drawViewOff(int viewX, int viewY, bool alpha)
{
    if(done()){
        return;
    }

    if(m_gfxEntry->gfxID == SYS_TEXNIL){
        return;
    }

    int offX = 0;
    int offY = 0;
    if(auto texPtr = g_magicDB->Retrieve(m_gfxEntry->gfxID + frame(), &offX, &offY)){
        SDLDevice::EnableTextureBlendMode enableBlendMode(texPtr, SDL_BLENDMODE_BLEND);
        SDLDevice::EnableTextureModColor enableModColor(texPtr, colorf::RGBA(0XFF, 0XFF, 0XFF, alpha ? 0X80 : 0XFF));
        g_SDLDevice->drawTexture(texPtr, m_x * SYS_MAPGRIDXP - viewX + offX, m_y * SYS_MAPGRIDYP - viewY + offY);
    }
}