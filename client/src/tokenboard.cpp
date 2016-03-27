/*
 * =====================================================================================
 *
 *       Filename: tokenboard.cpp
 *        Created: 06/17/2015 10:24:27 PM
 *  Last Modified: 03/26/2016 17:31:51
 *
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

#include "emoticon.hpp"
#include "emoticondbn.hpp"
#include "fontexdbn.hpp"
#include "section.hpp"
#include "tokenboard.hpp"
#include "tokenbox.hpp"
#include "utf8char.hpp"
#include "mathfunc.hpp"

#include <SDL2/SDL.h>
#include <algorithm>
#include <functional>
#include <tinyxml2.h>
#include <utf8.h>
#include <unordered_map>
#include <string>
#include <cassert>

bool TokenBoard::ParseXML(const char *szXML, 
        const std::unordered_map<std::string, std::function<void()>> &rstIDHandleMap)
{
    tinyxml2::XMLDocument stDoc;
    return !stDoc.Parse(szXML) && Load(stDoc, rstIDHandleMap);
}

bool TokenBoard::Load(const tinyxml2::XMLDocument &rstDoc,
        const std::unordered_map<std::string, std::function<void()>> &rstIDHandleMap)
{
    // 1. clear all informaiton
    //
    m_LineV.clear();
    m_IDFuncV.clear();
    m_SectionV.clear();
    m_LineStartY.clear();
    m_TokenBoxBitmap.clear();

    // 2. set variables
    m_MaxH1            = 0;
    m_MaxH2            = 0;
    m_W                = 0;
    m_H                = 0;
    m_SelectState      = 0;
    m_CurrentWidth     = 0;
    m_CurrentLineMaxH2 = 0;
    m_SkipEvent        = false;
    m_LastTokenBox     = nullptr;

    const tinyxml2::XMLElement *pRoot = rstDoc.RootElement();
    if (pRoot == nullptr){ return false; }

    const tinyxml2::XMLElement *pCurrentObject = nullptr;
    if(false){
    }else if((pCurrentObject = pRoot->FirstChildElement("object"))){
    }else if((pCurrentObject = pRoot->FirstChildElement("Object"))){
    }else if((pCurrentObject = pRoot->FirstChildElement("OBJECT"))){
    }else{
        return false;
    }

    int nSection = 0;

    ResetCurrentLine();

    while(pCurrentObject){
        if(ObjectReturn(*pCurrentObject)){
            // so for description like 
            // <object type = "return"></object><object type = "return"></object>
            // we can only get one new line
            if(!m_CurrentLine.empty()){
                AddNewTokenBoxLine(true);
            }
        }else if(ObjectEmocticon(*pCurrentObject)){
            ParseEmoticonObject(*pCurrentObject, nSection++, rstIDHandleMap);
        }else if(ObjectEventText(*pCurrentObject)){
            ParseEventTextObject(*pCurrentObject, nSection++, rstIDHandleMap);
        }else{
            // failed to parse something
        }
        pCurrentObject = NextObject(pCurrentObject);
    }

    if(!m_CurrentLine.empty()){
        AddNewTokenBoxLine(true);
    }

    if(!m_SkipEvent){
        MakeEventTextBitmap();
    }

    return true;
}

bool TokenBoard::ObjectReturn(const tinyxml2::XMLElement &rstCurrentObject)
{
    return false
        || (rstCurrentObject.Attribute("TYPE", "RETURN"))
        || (rstCurrentObject.Attribute("TYPE", "Return"))
        || (rstCurrentObject.Attribute("TYPE", "return"))
        || (rstCurrentObject.Attribute("Type", "RETURN"))
        || (rstCurrentObject.Attribute("Type", "Return"))
        || (rstCurrentObject.Attribute("Type", "return"))
        || (rstCurrentObject.Attribute("type", "RETURN"))
        || (rstCurrentObject.Attribute("type", "Return"))
        || (rstCurrentObject.Attribute("type", "return"));
}

bool TokenBoard::ObjectEmocticon(const tinyxml2::XMLElement &rstCurrentObject)
{
    return false
        || (rstCurrentObject.Attribute("TYPE", "Emoticon"))
        || (rstCurrentObject.Attribute("TYPE", "emoticon"))
        || (rstCurrentObject.Attribute("TYPE", "EMOTICON"))
        || (rstCurrentObject.Attribute("Type", "Emoticon"))
        || (rstCurrentObject.Attribute("Type", "emoticon"))
        || (rstCurrentObject.Attribute("Type", "EMOTICON"))
        || (rstCurrentObject.Attribute("type", "Emoticon"))
        || (rstCurrentObject.Attribute("type", "emoticon"))
        || (rstCurrentObject.Attribute("type", "EMOTICON"));
}

bool TokenBoard::ObjectEventText(const tinyxml2::XMLElement &rstCurrentObject)
{
    return false
        || (rstCurrentObject.Attribute("Type") == nullptr)
        || (rstCurrentObject.Attribute("type") == nullptr)
        || (rstCurrentObject.Attribute("TYPE") == nullptr)
        || (rstCurrentObject.Attribute("TYPE", "EVENTTEXT"))
        || (rstCurrentObject.Attribute("TYPE", "EventText"))
        || (rstCurrentObject.Attribute("TYPE", "Eventtext"))
        || (rstCurrentObject.Attribute("TYPE", "eventText"))
        || (rstCurrentObject.Attribute("TYPE", "eventtext"))
        || (rstCurrentObject.Attribute("Type", "EVENTTEXT"))
        || (rstCurrentObject.Attribute("Type", "EventText"))
        || (rstCurrentObject.Attribute("Type", "Eventtext"))
        || (rstCurrentObject.Attribute("Type", "eventText"))
        || (rstCurrentObject.Attribute("Type", "eventtext"))
        || (rstCurrentObject.Attribute("type", "EVENTTEXT"))
        || (rstCurrentObject.Attribute("type", "EventText"))
        || (rstCurrentObject.Attribute("type", "Eventtext"))
        || (rstCurrentObject.Attribute("type", "eventText"))
        || (rstCurrentObject.Attribute("type", "eventtext"));
}

int TokenBoard::GetEmoticonSet(const tinyxml2::XMLElement &rstCurrentObject)
{
    const char *pText = nullptr;

    if(false){
    }else if((pText = rstCurrentObject.Attribute("SET"))){
    }else if((pText = rstCurrentObject.Attribute("Set"))){
    }else if((pText = rstCurrentObject.Attribute("set"))){
    }else{
        pText = "0";
    }
    // here means when set can not be found or conversion fails
    // return 0
    return std::atoi(pText);
}

int TokenBoard::GetEmoticonIndex(const tinyxml2::XMLElement &rstCurrentObject)
{
    const char *pText = nullptr;

    if(false){
    }else if((pText = rstCurrentObject.Attribute("INDEX"))){
    }else if((pText = rstCurrentObject.Attribute("Index"))){
    }else if((pText = rstCurrentObject.Attribute("index"))){
    }else{
        pText = "0";
    }
    return std::atoi(pText);
}

const tinyxml2::XMLElement *TokenBoard::NextObject(const tinyxml2::XMLElement *pCurrentObject)
{
    const tinyxml2::XMLElement *pRet = nullptr;
    if(false){
    }else if((pRet = pCurrentObject->NextSiblingElement("OBJECT"))){
    }else if((pRet = pCurrentObject->NextSiblingElement("Object"))){
    }else if((pRet = pCurrentObject->NextSiblingElement("object"))){
    }else{
        pRet = nullptr;
    }
    return pRet;
}

int TokenBoard::GetFontSize(const tinyxml2::XMLElement &rstCurrentObject)
{
    const char *pText            = nullptr;
    const int   nDefaultFontSize = 18;

    if(false){
    }else if((pText = rstCurrentObject.Attribute("SIZE"))){
    }else if((pText = rstCurrentObject.Attribute("Size"))){
    }else if((pText = rstCurrentObject.Attribute("size"))){
    }else{
        return nDefaultFontSize;
    }
    int nFontSize = std::atoi(pText);
    return nFontSize == 0 ? nDefaultFontSize : nFontSize;
}

int TokenBoard::GetFontIndex(const tinyxml2::XMLElement &rstCurrentObject)
{
    const char *pText;

    if(false){
    }else if((pText = rstCurrentObject.Attribute("FONT"))){
    }else if((pText = rstCurrentObject.Attribute("Font"))){
    }else if((pText = rstCurrentObject.Attribute("font"))){
    }else{
        // default to set FontIndex to be zero
        pText = "0";
    }
    return std::atoi(pText);
}

int TokenBoard::GetFontStyle(const tinyxml2::XMLElement &rstCurrentObject)
{
    const char *pText;

    if(false){
    }else if((pText = rstCurrentObject.Attribute("STYLE"))){
    }else if((pText = rstCurrentObject.Attribute("Style"))){
    }else if((pText = rstCurrentObject.Attribute("style"))){
    }else{
        pText = "0";
    }

    return std::atoi(pText);
}

SDL_Color TokenBoard::GetEventTextCharColor(const tinyxml2::XMLElement &rstCurrentObject, int nEvent)
{
    if(nEvent < 0 || nEvent > 2){
        // for non-event text, try to parse ``color" attribute
        // if failed, return pure white
        return GetEventTextCharColor(rstCurrentObject);
    }

    const char *attrText[3][3] = {
        {"OFF",  "Off",  "off" },
        {"ON",   "On",   "on"  },
        {"PUSH", "Push", "push"}
    };

    SDL_Color defaultColor[3] = {
        { 0XFF, 0XFF, 0X00, 0XFF },
        { 0X00, 0XFF, 0X00, 0XFF },
        { 0XFF, 0X00, 0X00, 0XFF }
    };
    const char *pText;
    SDL_Color   color;

    if(false){
    }else if((pText = rstCurrentObject.Attribute(attrText[nEvent][0]))){
    }else if((pText = rstCurrentObject.Attribute(attrText[nEvent][1]))){
    }else if((pText = rstCurrentObject.Attribute(attrText[nEvent][2]))){
    }else{
        return defaultColor[nEvent];
    }

    color.r = 0x00;
    color.g = 0x00;
    color.b = 0x00;
    color.a = 0xff;

    if(false
            || !std::strcmp(pText, "RED")
            || !std::strcmp(pText, "Red")
            || !std::strcmp(pText, "red")){
        color.r = 0xff;
    }else if(false
            || !std::strcmp(pText, "GREEN")
            || !std::strcmp(pText, "Green")
            || !std::strcmp(pText, "green")){
        color.g = 0xff;
    }else if(false
            || !std::strcmp(pText, "BLUE")
            || !std::strcmp(pText, "Blue")
            || !std::strcmp(pText, "blue")){
        color.b = 0xff;
    }else if(false
            || !std::strcmp(pText, "YELLOW")
            || !std::strcmp(pText, "Yellow")
            || !std::strcmp(pText, "yellow")){
        color.r = 0xff;
        color.g = 0xff;
    }else{
        // TODO 0xab11df11
    }
    return color;
}

// no event version, try to parse ``color" attribute for non-event text
SDL_Color TokenBoard::GetEventTextCharColor(const tinyxml2::XMLElement &rstCurrentObject)
{
    const char *pText;
    SDL_Color   color;

    if(false){
    }else if((pText = rstCurrentObject.Attribute("COLOR"))){
    }else if((pText = rstCurrentObject.Attribute("Color"))){
    }else if((pText = rstCurrentObject.Attribute("color"))){
    }else{
    }
    if(pText == nullptr){
        color.r = 0xff;
        color.g = 0xff;
        color.b = 0xff;
        color.a = 0xff;
        return color;
    }

    color.r = 0x00;
    color.g = 0x00;
    color.b = 0x00;
    color.a = 0xff;
    if(false
            || !std::strcmp(pText, "RED")
            || !std::strcmp(pText, "Red")
            || !std::strcmp(pText, "red")){
        color.r = 0xff;
    }else if(false
            || !std::strcmp(pText, "GREEN")
            || !std::strcmp(pText, "Green")
            || !std::strcmp(pText, "green")){
        color.g = 0xff;
    }else if(false
            || !std::strcmp(pText, "BLUE")
            || !std::strcmp(pText, "Blue")
            || !std::strcmp(pText, "blue")){
        color.b = 0xff;
    }else{
        // TODO 0xab11df11
    }
    return color;
}

bool TokenBoard::ParseEmoticonObject(const tinyxml2::XMLElement &rstCurrentObject,
        int/*nSection*/, const std::unordered_map<std::string, std::function<void()>> &/*rstIDHandleMap*/)
{
    // currently I didn't support event response for emoticon box
    // since my idea is not ready for it
    // so just put a null handler here
    m_IDFuncV.push_back(std::function<void()>());

    SECTION  stSection;
    TOKENBOX stTokenBox;
    std::memset(&stSection , 0, sizeof(stSection) );
    std::memset(&stTokenBox, 0, sizeof(stTokenBox));

    stSection.Info.Type                 = SECTIONTYPE_EMOTICON;
    stSection.State.Emoticon.FrameIndex = 0;
    stSection.State.Emoticon.MS         = 0.0;
    stSection.Info.Emoticon.Set         = GetEmoticonSet(rstCurrentObject);
    stSection.Info.Emoticon.Index       = GetEmoticonIndex(rstCurrentObject);

    extern EmoticonDBN *g_EmoticonDBN;
    auto pTexture = g_EmoticonDBN->Retrieve(
            stSection.Info.Emoticon.Set,          // emoticon set
            stSection.Info.Emoticon.Index,        // emoticon index
            0,                                    // first frame of the emoticon
            nullptr,                              // don't need the start info here
            nullptr,                              // don't need the satrt info here
            &stTokenBox.Cache.W,
            &stTokenBox.Cache.H,
            &stSection.Info.Emoticon.FPS,         // o
            &stTokenBox.Cache.H1,                 // o
            &stSection.Info.Emoticon.FrameCount); // o

    if(pTexture){
        // this emoticon is valid
        stTokenBox.Cache.H1            = stTokenBox.Cache.H - stTokenBox.Cache.H2;
        stTokenBox.State.Valid         = 1;
        stSection.State.Emoticon.Valid = 1;
    }

    m_SectionV.push_back(stSection);
    return AddNewTokenBox(stTokenBox);
}

bool TokenBoard::ParseEventTextObject(const tinyxml2::XMLElement &rstCurrentObject,
        int nSection, const std::unordered_map<std::string, std::function<void()>> &rstIDHandleMap)
{
    SECTION stSection;
    std::memset(&stSection, 0, sizeof(stSection));

    // parse event text section desc
    stSection.Info.Type           = SECTIONTYPE_EVENTTEXT;
    stSection.Info.Text.Font = GetFontIndex(rstCurrentObject);
    stSection.Info.Text.Style     = GetFontStyle(rstCurrentObject);
    stSection.Info.Text.Size      = GetFontSize(rstCurrentObject);

    std::function<void()> fnCallback;
    const char *szID = rstCurrentObject.Attribute("ID");

    if(szID){
        // add a handler here
        auto pFuncInst = rstIDHandleMap.find(std::string(szID));
        if(pFuncInst != rstIDHandleMap.end()){
            fnCallback = pFuncInst->second;
        }
    }

    m_IDFuncV.push_back(fnCallback);

    stSection.Info.Text.Color[0] = GetEventTextCharColor(rstCurrentObject, (szID) ? 0 : (-1));
    stSection.Info.Text.Color[1] = GetEventTextCharColor(rstCurrentObject, (szID) ? 1 : (-1));
    stSection.Info.Text.Color[2] = GetEventTextCharColor(rstCurrentObject, (szID) ? 2 : (-1));

    stSection.State.Text.Event = 0;
    m_SectionV.push_back(stSection);

    // then we parse event text content
    const char *pStart = rstCurrentObject.GetText();
    const char *pEnd   = pStart;

    // when get inside this funciton
    // the section structure has been well-prepared
    //
    int nDefaultSize = (int)m_SectionV[nSection].Info.Text.Size;

    uint32_t nFontAttrKey = 0
        + (((uint32_t)m_SectionV[nSection].Info.Text.Font) << 16)
        + (((uint32_t)m_SectionV[nSection].Info.Text.Size)      <<  8)
        + (((uint32_t)m_SectionV[nSection].Info.Text.Style)     <<  0);


    while(*pEnd != '\0'){
        pStart = pEnd;
        utf8::unchecked::advance(pEnd, 1);

        // should be true
        assert(pEnd - pStart <= 4);

        uint32_t nUTF8Key = 0;
        std::memcpy(&nUTF8Key, pStart, pEnd - pStart);

        // fully set the token box unit
        TOKENBOX stTokenBox;
        std::memset(&stTokenBox, 0, sizeof(stTokenBox));

        uint64_t nKey = (((uint64_t)nFontAttrKey << 32) + nUTF8Key);

        stTokenBox.Section = nSection;
        stTokenBox.UTF8CharBox.UTF8Code  = nUTF8Key;
        stTokenBox.UTF8CharBox.Cache.Key = nKey;

        extern FontexDBN *g_FontexDBN;
        auto pTexture = g_FontexDBN->Retrieve(nKey);
        if(pTexture){
            SDL_QueryTexture(pTexture, nullptr, nullptr, &stTokenBox.Cache.W, &stTokenBox.Cache.H);
            stTokenBox.Cache.H1    = stTokenBox.Cache.H;
            stTokenBox.Cache.H2    = 0;
            stTokenBox.State.Valid = 1;
        }else{
            // failed to retrieve, set a []
            stTokenBox.Cache.W  = nDefaultSize;
            stTokenBox.Cache.H  = nDefaultSize;
            stTokenBox.Cache.H1 = nDefaultSize;
            stTokenBox.Cache.H2 = 0;
        }
        if(!AddNewTokenBox(stTokenBox)){
            return false;
        }
    }

    return true;
}

void TokenBoard::UpdateSection(SECTION &stSection, Uint32 ticks)
{
    // only emoticon needs to be updated
    // but we add API here for further
    switch(stSection.Info.Type){
        case SECTIONTYPE_EMOTICON:
            {
                Uint32 nMaxTicks     = (Uint32)std::lround(1000.0 / stSection.Info.Emoticon.FPS);
                Uint32 nCurrentTicks = (Uint32)std::lround(1.0 * ticks + stSection.State.Emoticon.MS);
                int    nFrameIndex   = stSection.State.Emoticon.FrameIndex + nCurrentTicks / nMaxTicks;

                stSection.State.Emoticon.FrameIndex = nFrameIndex % stSection.Info.Emoticon.FrameCount;
                stSection.State.Emoticon.MS      = nCurrentTicks % nMaxTicks;
                break;
            }
        default:
            break;
    }
}

void TokenBoard::Update(double fMS)
{
    for(auto &stSection: m_SectionV){
        UpdateSection(stSection, fMS);
    }
}

int TokenBoard::SectionTypeCount(const std::vector<TOKENBOX> & vLine, int nSectionType)
{
    int nCount = 0;
    if(nSectionType != 0){
        for(const auto &stTokenBox: vLine){
            if(m_SectionV[stTokenBox.Section].Info.Type & nSectionType){
                nCount++;
            }
        }
        return nCount;
    }else{
        return m_CurrentLine.size();
    }
}

int TokenBoard::DoPadding(int nDWidth, int nSectionType)
{
    //
    // ++----------+------------+
    // |+        + |
    // ||        | |
    // |+        + |
    // +-----------+------------+
    //
    //  nDWidth      : recommended width
    //  nSectionType : specify SectionType for padding
    //
    // return should always be non-negative
    // meaning we can't be out of the boundary
    //
    // only do padding, won't calculate StartX/Y for simplity

    int nCount = SectionTypeCount(m_CurrentLine, nSectionType);
    if(m_CurrentLine.size() > 1 && nCount != 0){
        int nPaddingSpace = nDWidth / nCount;

        // first round
        // first token may reserve for no padding
        if(TokenBoxType(m_CurrentLine[0]) != TokenBoxType(m_CurrentLine[1])){
            if(nSectionType == 0 || TokenBoxType(m_CurrentLine[0]) & nSectionType){
                m_CurrentLine[0].State.W1 = 0;
                m_CurrentLine[0].State.W2 = nPaddingSpace / 2;

                nDWidth        -= nPaddingSpace / 2;
                m_CurrentWidth += nPaddingSpace / 2;
                if(nDWidth == 0){
                    return 0;
                }
            }
        }

        for(size_t nIndex = 1; nIndex < m_CurrentLine.size() - 1; ++nIndex){
            if(nSectionType == 0 || TokenBoxType(m_CurrentLine[nIndex]) & nSectionType){
                m_CurrentLine[nIndex].State.W1 = nPaddingSpace / 2;
                m_CurrentLine[nIndex].State.W2 = nPaddingSpace - nPaddingSpace / 2;

                nDWidth        -= nPaddingSpace;
                m_CurrentWidth += nPaddingSpace;
                if(nDWidth == 0){
                    return 0;
                }
            }
        }

        if(nSectionType == 0 || TokenBoxType(m_CurrentLine.back()) & nSectionType){
            m_CurrentLine.back().State.W1 = nPaddingSpace / 2;
            m_CurrentLine.back().State.W2 = 0;

            nDWidth        -= nPaddingSpace / 2;
            m_CurrentWidth += nPaddingSpace / 2;
            if(nDWidth == 0){
                return 0;
            }
        }

        // second round
        if(TokenBoxType(m_CurrentLine[0]) != TokenBoxType(m_CurrentLine[1])){
            if(nSectionType == 0 || TokenBoxType(m_CurrentLine[0]) & nSectionType){
                m_CurrentLine[0].State.W2++;;
                nDWidth--;
                m_CurrentWidth++;
                if(nDWidth == 0){
                    return 0;
                }
            }
        }

        for(size_t nIndex = 1; nIndex < m_CurrentLine.size() - 1; ++nIndex){
            if(nSectionType == 0 || TokenBoxType(m_CurrentLine[nIndex]) & nSectionType){
                if(m_CurrentLine[nIndex].State.W1 <=  m_CurrentLine[nIndex].State.W2){
                    m_CurrentLine[nIndex].State.W1++;
                }else{
                    m_CurrentLine[nIndex].State.W2++;
                }
                nDWidth--;
                m_CurrentWidth++;
                if(nDWidth == 0){
                    return 0;
                }
            }
        }

        if(nSectionType == 0 || TokenBoxType(m_CurrentLine.back()) & nSectionType){
            m_CurrentLine.back().State.W1++;
            nDWidth--;
            m_CurrentWidth++;
            if(nDWidth == 0){
                return 0;
            }
        }
    }
    return nDWidth;
}

void TokenBoard::SetTokenBoxStartY(std::vector<TOKENBOX> &vTokenBox, int nBaseLineY)
{
    for(auto &stTokenBox: vTokenBox){
        stTokenBox.Cache.StartY = nBaseLineY - stTokenBox.Cache.H1;
    }
}

void TokenBoard::SetTokenBoxStartX(std::vector<TOKENBOX> &vTokenBox)
{
    int nCurrentX = 0;
    for(auto &stTokenBox: vTokenBox){
        nCurrentX += stTokenBox.State.W1;
        stTokenBox.Cache.StartX = nCurrentX;
        nCurrentX += stTokenBox.Cache.W;
        nCurrentX += stTokenBox.State.W2;
    }
}

int TokenBoard::SpacePadding(int paddingWidth)
{
    // return the real line width after padding
    // if paddingWidth < m_CurrentWidth
    // this function will do nothing and return m_CurrentWidth
    //
    // +----+-------------------+----+
    // |    |                   |    |
    // |    | ..........        |    |
    // +----+-------------------+----+

    int nDWidth = paddingWidth - (m_CurrentWidth + 1);
    if(nDWidth > 0){
        // try to padding all by emoticons
        int newDWidth = DoPadding(nDWidth, SECTIONTYPE_EMOTICON);
        if(newDWidth == nDWidth){
            // doesn't work, padding by all tokenbox
            newDWidth = DoPadding(nDWidth, 0);
            return m_CurrentWidth;
        }
    }
    return m_CurrentWidth;
}

bool TokenBoard::AddTokenBox(TOKENBOX &stTokenBox)
{
    // Emoticon itself has a box
    // and need to align to the baseline of words
    //                             
    //           +----------+             -----
    //           |          |               |  
    // +-------+ |          |               | H1
    // | Words | | Emoticon |  Words        |    
    // +-------+-+----------+---------------|--
    //           |          |               | H2
    //           +----------+             -----
    //                             

    if(m_PW > 0 && m_CurrentWidth + stTokenBox.Cache.W >= m_PW){
        // when wrapping, width control is enabled
        return false;
    }else{
        stTokenBox.State.W1 = 0;
        stTokenBox.State.W2 = 0;
        // here it's copy
        m_CurrentLine.push_back(stTokenBox);
        m_CurrentWidth    += stTokenBox.Cache.W;
        m_CurrentLineMaxH2 = (std::max)(m_CurrentLineMaxH2, stTokenBox.Cache.H2);
        return true;
    }
}

void TokenBoard::ResetCurrentLine()
{
    m_CurrentLine.clear();
    m_CurrentWidth     = 0;
    m_CurrentLineMaxH2 = 0;
}

int TokenBoard::GetNthLineIntervalMaxH2(int nthLine, int nIntervalStartX, int nIntervalWidth)
{
    //    This function only take care of nthLine
    //    has nothing to do with (n-1)th or (n+1)th Line
    //    TODO: we ignore the following case here:
    //
    // ---------+----+----+-------------+-------  <-BaseLine
    // |        |    |    |             |    
    // |        | W2 | W1 |             |       
    // |        |    +----+-------------+
    // |        |    |
    // +--------+----+                     
    //             |    |
    //             |    |
    //             |    |
    //        +----+----+-----+
    //        | W1 |    | W2  |
    //        |  ->|    |<-   |
    //            Interval
    //
    //  Here we take the padding space as part of the token box.
    //  If not, token in (n + 1)-th line may go through this space
    //
    //  keep this as it is, and update it later

    int   nMaxH2    = -1;
    int   nCurrentX =  0;

    for(auto &stTokenBox: m_LineV[nthLine]){
        int nW  = stTokenBox.Cache.W;
        int nH2 = stTokenBox.Cache.H2;
        int nW1 = stTokenBox.State.W1;
        int nW2 = stTokenBox.State.W2;

        if(IntervalOverlap(nCurrentX, nCurrentX + nW1 + nW + nW2,
                    nIntervalStartX, nIntervalStartX + nIntervalWidth)){
            nMaxH2 = (std::max)(nMaxH2, nH2);
        }
        nCurrentX += (nW1 + nW + nW2);
    }
    // maybe the line is not long enough to cover Interval
    // in this case we return -1
    return nMaxH2;
}

int TokenBoard::GetNthLineTokenBoxStartY(int nthLine, int nStartX, int nBoxWidth, int nBoxHeight)
{
    while(nthLine - 1 >= 0){
        int nMaxH2 = GetNthLineIntervalMaxH2(nthLine - 1, nStartX, nBoxWidth);
        if(nMaxH2 >= 0){
            return m_LineStartY[nthLine - 1] + nBoxHeight + nMaxH2;
        }
        nthLine--;
    }
    // there is no line upside
    return nBoxHeight;
}

int TokenBoard::GetNthNewLineStartY(int nthLine)
{
    // m_CurrentLine is padded already
    //
    //                             
    //           +----------+                  
    //           |          |                  
    // +-------+ |          |                   
    // | Words | | Emoticon |  Words             
    // +-------+-+----------+------------------
    //           |          |   +----------+  
    //           +----------+   |          |
    //                          |          |                  
    //                +-------+ |          |                   
    //                | Words | | Emoticon |  Words             
    //                +-------+-+----------+-------           
    //                          |          |                   
    //                          +----------+                  
    //                                            
    //
    int nCurrentX =  0;
    int nCurrentY = -1;
    for(auto &stTokenBox: m_CurrentLine){
        int nW  = stTokenBox.Cache.W;
        int nH1 = stTokenBox.Cache.H1;
        int nW1 = stTokenBox.State.W1;
        int nW2 = stTokenBox.State.W2;

        nCurrentY  = (std::max)(nCurrentY,
                GetNthLineTokenBoxStartY(nthLine, nCurrentX + nW1, nW, nH1));
        nCurrentX += (nW1 + nW + nW2);
    }
    return nCurrentY;
}

// everything dynamic and can be retrieve is in Cache
//            dynamic but can not be retrieved is in State
//            static is in Info
void TokenBoard::DrawEx(
        int nDstX, int nDstY, // start position of drawing on the screen
        int nSrcX, int nSrcY, // region to draw, a cropped region on the token board
        int nSrcW, int nSrcH)
{
    // we assume all token box are well-prepared here!
    // means cache, state, info are all valid now when invoke this function
    //

    // if(false
    //         || nSrcX >= m_W
    //         || nSrcY >= m_H
    //         || nSrcX + nDstW <= 0
    //         || nSrcY + nDstH <= 0){
    //     return;
    // }
    //
    // if(nSrcX < 0){
    //     nSrcW += nSrcX;
    //     nSrcX  = 0;
    // }
    //
    // if(nSrcY < 0){
    //     nSrcH += nSrcY;
    //     nSrcY  = 0;
    // }
    //
    // if(nSrcX + nSrcW > m_W){
    //     nSrcW = m_W - nSrcX;
    // }
    //
    // if(nSrcY + nSrcH > m_H){
    //     nSrcH = m_H - nSrcY;
    // }

    // now nSrc(X, Y, W, H) is a sub-area on the board

    // we need to check it since for each tokenbox, overlap checking is expensive
    bool bCheckInside = !(nSrcX == 0 && nSrcY == 0 && nSrcW == W() && nSrcH == H());

    for(int nLine = 0; nLine < (int)m_LineV.size(); ++nLine){
        for(auto &rstTokenBox: m_LineV[nLine]){

            // get tokenbox size info, this is uniform for Text/Emoticon
            int nX, nY, nW, nH;
            nX = rstTokenBox.Cache.StartX;
            nY = rstTokenBox.Cache.StartY;
            nW = rstTokenBox.Cache.W - rstTokenBox.State.W1 - rstTokenBox.State.W2;
            nH = rstTokenBox.Cache.H;

            bool bCheckOverlap = bCheckInside && !RectangleInside(0, 0, W(), H(), nX, nY, nW, nH);

            if(bCheckOverlap && !RectangleOverlapRegion(
                        nSrcX, nSrcY, nSrcW, nSrcH, &nX, &nY, &nW, &nH)){
                // need to check overlap and it's outside
                continue;
            }

            switch(m_SectionV[rstTokenBox.Section].Info.Type){
                case SECTIONTYPE_EVENTTEXT:
                    {
                        // I had a hard time here
                        //
                        // TODO
                        //
                        // the purpose of introducing FontexDB is for saving video memory
                        // not to support hot source data switch!
                        //
                        // assumptions:
                        //  1. if a fontex can't be retrieved, it can never be retrieved
                        //     that's to say, the data source is fixed
                        //
                        //  2. if a fontex can be retrieved, then it can be retrieved everytime
                        //     the fontex release and restore are transparant to token board
                        //
                        //  3. if a token box cache is invalid, we need to set it at place
                        //     but if it's valid, it always matches the data
                        //
                        //     that to say, the cache data set by fontex are always the same
                        //     we can't steal the data source and replace by a new one at runtime
                        //

                        int nEvent = m_SectionV[rstTokenBox.Section].State.Text.Event;
                        auto &rstColor = m_SectionV[nEvent].Info.Text.Color[nEvent];

                        extern SDLDevice *g_SDLDevice;
                        extern FontexDBN *g_FontexDBN;
                        auto pTexture = g_FontexDBN->Retrieve(rstTokenBox.UTF8CharBox.Cache.Key);

                        if(pTexture){
                            SDL_SetTextureColorMod(pTexture, rstColor.r, rstColor.g, rstColor.b);
                            int nDX = nX - rstTokenBox.Cache.StartX;
                            int nDY = nY - rstTokenBox.Cache.StartY;
                            g_SDLDevice->DrawTexture(pTexture,
                                    nX + nDstX, nY + nDstY, nDX, nDY, nW, nH);
                        }else{
                            // TODO
                            // draw a box here to indicate errors
                        }

                        break;
                    }

                case SECTIONTYPE_EMOTICON:
                    {
                        int nFrameIndex = m_SectionV[rstTokenBox.Section].State.Emoticon.FrameIndex;
                        extern EmoticonDBN *g_EmoticonDBN;
                        extern SDLDevice   *g_SDLDevice;

                        int nXOnTex, nYOnTex;
                        auto pTexture = g_EmoticonDBN->Retrieve(
                                rstTokenBox.EmoticonBox.Cache.Key + nFrameIndex,
                                &nXOnTex, &nYOnTex,
                                nullptr, nullptr, nullptr, nullptr, nullptr);

                        if(pTexture){
                            int nDX = nX - rstTokenBox.Cache.StartX;
                            int nDY = nY - rstTokenBox.Cache.StartY;
                            g_SDLDevice->DrawTexture(pTexture,
                                    nX + nDstX, nY + nDstY, nXOnTex + nDX, nYOnTex + nDY, nW, nH);
                        }else{
                            // TODO
                            // draw a box to indicate errors
                        }

                        break;
                    }
                default:
                    break;
            }
        }
    }
}

void TokenBoard::AddNewTokenBoxLine(bool bEndByReturn)
{
    if(m_PW > 0 && bEndByReturn == false){
        SpacePadding(m_PW);
    }

    // set X cache
    SetTokenBoxStartX(m_CurrentLine);

    // always align lines to left
    // otherwise we need m_LineStartX
    // but GetNthNewLineStartY() need TOKENBOX::Cache.StartX
    // kind of inconsistent

    m_LineStartY.push_back(GetNthNewLineStartY(m_LineV.size()));

    // set Y cache
    SetTokenBoxStartY(m_CurrentLine, m_LineStartY.back());

    m_LineV.emplace_back(std::move(m_CurrentLine));

    // for wrap or not
    m_W = (std::max)(m_W, m_CurrentWidth);
    m_H = m_LineStartY.back() + m_CurrentLineMaxH2 + 1;

    // this is a must for AddNewTokenBox() to end the recursion
    ResetCurrentLine();
}

bool TokenBoard::AddNewTokenBox(TOKENBOX &rstTokenBox)
{
    if(AddTokenBox(rstTokenBox)){
        return true;
    }else{
        // try to add but failed
        if(m_CurrentLine.empty()){
            // allowed width is tooo small
            // won't fix it, just fail and leave
            return false;
        }else{
            // 1. insert a finished line
            // 2. recursively call itself to add the token again
            AddNewTokenBoxLine(false);
            // the depth of recursion at most is 2
            return AddNewTokenBox(rstTokenBox);
        }
    }
}


void TokenBoard::TokenBoxGetMouseButtonUp(const TOKENBOX &rstTokenBox, bool bFirstHalf)
{
    switch(m_SectionV[rstTokenBox.Section].Info.Type){
        case SECTIONTYPE_PLAINTEXT:
        case SECTIONTYPE_EMOTICON:
            {
                // for plain text and emoticon, we only need to think about dragging
                // and it can't accept other kind of events
                if(m_Dragging){
                    // 1. stop the dragging
                    m_Dragging = false;
                    // 2. record the end offset of dragging
                    m_DraggingStop.first  = rstTokenBox.Section;
                    m_DraggingStop.second = rstTokenBox.Offset - (bFirstHalf ? 1 : 0);
                }else{
                    // this should be click, need to do nothing
                }
                break;
            }
        case SECTIONTYPE_EVENTTEXT:
            {
                // 1. may trigger event
                // 2. may be the end of dragging
                switch(m_SectionV[.rstTokenBox.Section].State.Text.Event){
                    case 0:
                        {
                            // impossible generally, only possibility comes with
                            // user are moving mouse *very* fast. Then pointer can
                            // jump inside one tokenbox and emit button up event.
                            //
                            // do nothing
                            break;
                        }
                    case 1:
                        {
                            // over state and then get release, this only can be dragging
                            if(m_Dragging){
                                // 1. stop the dragging
                                m_Dragging = false;
                                // 2. record the end offset of dragging
                                m_DraggingStop.first  = rstTokenBox.Section;
                                m_DraggingStop.second = rstTokenBox.Offset - (bFirstHalf ? 1 : 0);
                            }else{
                                // impossible
                            }
                            break;
                        }
                    case 2:
                        {
                            // pressed state, and get released event, need tirgger
                            // 1. make as state ``over"
                            m_SectionV[m_LastTokenBox->Section].State.Text.Event = 1;
                            // 2. trigger registered event handler
                            if(m_IDFuncV[m_LastTokenBox->Section]){
                                m_IDFuncV[m_LastTokenBox->Section]();
                            }
                            break;
                        }
                    default:
                        {
                            break;
                        }

                }

                break;
            }
    }
}

void TokenBoard::TokenBoxGetMouseButtonDown(const TOKENBOX &rstTokenBox, bool bFirstHalf)
{
    switch(m_SectionV[rstTokenBox.Section].Info.Type){
        case SECTIONTYPE_PLAINTEXT:
        case SECTIONTYPE_EMOTICON:
            {
                // 1. always drop the selected result off
                m_SelectState = 0;

                std::pair<int, int> stCursorLoc = {
                    rstTokenBox.Section,
                    rstTokenBox.Offset - (bFirstHalf ? 1 : 0)
                };

                // 2. mark for the preparing of next selection
                if(m_Selectable){ m_DraggingStart = stCursorLoc; }
                if(m_WithCursor){ m_CursorLoc     = stCursorLoc; }

                break;
            }
        case SECTIONTYPE_EVENTTEXT:
            {
                // set to be pressed directly
                m_SectionV[.rstTokenBox.Section].State.Text.Event = 2;
                break;
            }
    }
}

void TokenBoard::TokenBoxGetMouseMotion(const TOKENBOX &rstTokenBox, bool bFirstHalf)
{
    if(m_Selectable){
        m_SelectState = 1;
        m_SelectStop = {
            rstTokenBox.Section,
            rstTokenBox.Offset - (bFirstHalf ? 1 : 0)
        };
    }
}

void TokenBoard::ProcessEventMouseButtonUp(int nEventDX, int nEventDY)
{

    if(!In(nEventDX, nEventDY)){ return; }

    if(m_LastTokenBox){
        // 
        // W1 and W2 should also count in
        // otherwise mouse can escape from the flaw of two contiguous tokens
        // means when move mouse horizontally, event text will turn on and off
        //
        int nX = m_LastTokenBox->Cache.StartX - m_LastTokenBox->State.W1;
        int nY = m_LastTokenBox->Cache.StartY;
        int nW = m_LastTokenBox->Cache.W;
        int nH = m_LastTokenBox->Cache.H;

        if(PointInRectangle(nEventDX, nEventDY, nX, nY, nW, nH)){
            TokenBoxGetMouseButtonUp(*m_LastTokenBox, nEventDX < nX + nW / 2 );
        }
    }else{
        int nGridX = nEventDX / m_Resolution;
        int nGridY = nEventDY / m_Resolution;

        // only put EventText Section in Bitmap
        // emoticon won't accept events
        //
        for(auto pTokenBox: m_TokenBoxBitmap[nGridX][nGridY]){
            //
            // for each possible tokenbox in the grid
            //
            // this is enough since cover of all tokenboxs in one gird
            // is much larger than the grid itself
            const auto &rstTokenBox = *pTokenBox;

            int nStartX = rstTokenBox.Cache.StartX - rstTokenBox.State.W1;
            int nStartY = rstTokenBox.Cache.StartY;
            int nW      = rstTokenBox.Cache.W;
            int nH      = rstTokenBox.Cache.H;

            if(PointInRectangle(nEventX, nEventY, nStartX, nStartY, nW, nH)){
                TokenBoxGetMouseButtonUp(rstTokenBox, nEventDX < nStartX + nW / 2);
            }
        }
    }
}

void TokenBoard::ProcessEvent(const SDL_Event &rstEvent, bool *bValid)
{
    // don't need to handle event or event has been consumed
    if(m_SkipEvent || (bValid && !(*bValid))){ return; }

    uint32_t nEventType;
    int nEventDX, nEventDY;
    switch(rstEvent.type){
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                nEventType = rstEvent.type;
                nEventDX = rstEvent.button.x - X();
                nEventDY = rstEvent.button.y - Y();
                break;
            }
        case SDL_MOUSEMOTION:
            {
                nEventType = rstEvent.type;
                nEventDX = rstEvent.motion.x - X();
                nEventDY = rstEvent.motion.y - Y();
                break;
            }
        default:
            {
                // TBD
                // for all other event, won't handle, just return
                // maybe key up/down event maybe handled in the future
                return;
            }
    }

    if(m_LastTokenBox){
        // 
        // W1 and W2 should also count in
        // otherwise mouse can escape from the flaw of two contiguous tokens
        // means when move mouse horizontally, event text will turn on and off
        //
        int nX = m_LastTokenBox->Cache.StartX - m_LastTokenBox->State.W1;
        int nY = m_LastTokenBox->Cache.StartY;
        int nW = m_LastTokenBox->Cache.W;
        int nH = m_LastTokenBox->Cache.H;

        if(PointInRectangle(nEventDX, nEventDY, nX, nY, nW, nH)){
            TokenBoxGetEvent(*m_LastTokenBox, nEventDX, nEventDY);
            return;
        }else{
            m_SectionV[m_LastTokenBox->Section].State.Text.Event = 0;
            m_LastTokenBox = nullptr;
        }
    }

    if(In(nEventDX, nEventDY)){
        int nGridX = nEventDX / m_Resolution;
        int nGridY = nEventDY / m_Resolution;

        for(auto pTokenBox: m_TokenBoxBitmap[nGridX][nGridY]){

            int nX = pTokenBox->Cache.StartX - pTokenBox->State.W1;
            int nY = pTokenBox->Cache.StartY;
            int nW = pTokenBox->Cache.W;
            int nH = pTokenBox->Cache.H;

            if(PointInRectangle(nEventX, nEventY, nX, nY, nW, nH)){
                TokenBoxGetEvent(pLastTokenBox, nEventDX, nEventDY);
                m_LastTokenBox = pTokenBox;
                // here we don't need to handle last tokenbox
                // since if we really need, it's already be handled before
                return;
            }
        }

        // no box can handle this event, need to check line by line
        if(m_Selectable || m_WithCursor){
            // consume this event anyway
            *bValid = false;

            int nRowIn = -1;
            for(int nRow = 0; nRow < m_LineStartY.size(); ++nRow){
                if(m_LineStartY[nRow] > m_LineStartY[nRow]){
                    // this may cause a problem:
                    //
                    //
                    // +-----------+ +----+
                    // |           | |    |
                    // +-----------+ |    | --------------- Y
                    //              *|    |
                    //     +-------+ |    |
                    //     |       | +----+
                    //     |       | +--+
                    //     |       | |  |
                    //     +-------+ +--+   --------------- Y
                    //
                    // now if click is at "*", it will give cursor at the second
                    // line, not the first line

                    nRowIn = nRow;
                    break;
                }
            }

            if(m_LineStartY.size() > 0 && nEventDY > m_LineStartY.back()){
                // after the last line we count it in the last line
                nRowIn = m_LineStartY.size() - 1;
            }

            if(nRowIn < 0){ return; }

            int nTokenBoxBind = -2; // here -1 is valid
            int nLastCX = -1;
            for(int nXCnt = 0; nXCnt < m_LineV[nRowIn].size(); ++nXCnt){
                int nX  = m_LineV[nRowIn][nXCnt].Cache.StartX;
                int nW  = m_LineV[nRowIn][nXCnt].Cache.W;
                int nCX = nX + nW / 2;

                if(PointInSegment(nEventDX, nLastCX, nCX - nLastCX + 1)){
                    nTokenBoxBind = nXCnt - 1;
                    break;
                }

                nLastCX = nCX;
            }

            if(nTokenBoxBind == -2){
                // if we can't bind, bind it to the last box
                // this may cause problem:
                //
                // +-----------+ +----+
                // |           | |    |
                // +-----------+ |    | --------------- Y
                //               |    |
                //     +-------+ |    |
                //     |       | |    |
                //     |       | |    |
                //     |       | |    |
                //     |       | |    | *
                //     |       | +----+
                //     +-------+ ---------------------- Y
                //
                // when clicking at "*", we get cursor bind
                // at the second line
                //
                nTokenBoxBind = m_LineV[nRowIn].size() - 1;
            }

            if(m_WithCursor){
                m_CursorLoc = {nRowIn, nTokenBoxBind};
            }

            if(m_Selectable){
                m_SelectStartLoc = {nRowIn, nTokenBoxBind};
                m_SelectState = 0;
            }
        }
    }
}

int TokenBoard::TokenBoxType(const TOKENBOX &rstTokenBox)
{
    return m_SectionV[rstTokenBox.Section].Info.Type;
}

void TokenBoard::MakeEventTextBitmap()
{
    // only make it for EventText since only it accepts event
    //
    // TODO
    // maybe emoticons will also accept events
    // but that introduces more issues
    //    1. what to show the difference for press/release/out?
    //    2. what frame index should be set when pressed?
    //    3. ...
    // this is too much, so just disable it in the design
    //

    int nGW = (W() + (m_Resolution - 1)) / m_Resolution;
    int nGH = (H() + (m_Resolution - 1)) / m_Resolution;

    // this should be easier to clarify
    m_TokenBoxBitmap.resize(nGW);
    for(int nGX = 0; nGX < nGW; ++nGX){
        m_TokenBoxBitmap[nGX].resize(nGH, {});
    }

    for(auto &rstLine: m_LineV){
        for(auto &rstTokenBox: rstLine){
            if(TokenBoxType(rstTokenBox) == SECTIONTYPE_EVENTTEXT){
                MarkEventTextBitmap(rstTokenBox);
            }
        }
    }
}

void TokenBoard::MarkEventTextBitmap(const TOKENBOX &rstTokenBox)
{
    // put the whole box (with the margin) to the bitmap
    // not only the real box inside
    //
    // E : emoticon
    // T : text
    //
    //              |<---Cache.W---->|
    //              |                |
    //    ---- ---- +---+--------+---+  <--- Cache.StartY
    //     ^    ^   |   |        |   |
    //     |    |   |   |        |   +---+-----+---+
    //     |   H1   |   |        |   |   |     |   |
    //     |    |   |   |   E    |   |   |  T  |   |
    //     H    |   |   |        |   |   |     |   |
    //     |   -x---+---+--------+---+---+-----+---+-             
    //     |    |   |   |        |   |
    //     |   H2   |   |        |   |
    //     V    V   |   |        |   |
    //    ---- -----+---+--------+---+
    //              ^   ^
    //              |   |
    //              |   |
    //              |   +--Cache.StartX
    //              +------Cache.StartX - State.W1

    // get the whole box's (x, y, w, h), with the margin
    //
    int nX = rstTokenBox.Cache.StartX - rstTokenBox.State.W1;
    int nY = rstTokenBox.Cache.StartY;
    int nW = rstTokenBox.Cache.W;
    int nH = rstTokenBox.Cache.H;

    // map it to all the grid box
    // maybe more than one
    int nGX1 = nX / m_Resolution;
    int nGY1 = nY / m_Resolution;
    int nGX2 = (nX + nW) / m_Resolution;
    int nGY2 = (nY + nH) / m_Resolution;

    for(int nX = nGX1; nX <= nGX2; ++nX){
        for(int nY = nGY1; nY <= nGY2; ++nY){
            m_TokenBoxBitmap[nX][nY].emplace_back(&rstTokenBox);
        }
    }
}

int TokenBoard::GuessResoltion()
{
    // TODO
    // make this function more reasonable and functional
    return (m_MaxSize + m_MinSize) / 2;
}

bool TokenBoard::ParseXML(const char *szText)
{}

std::string TokenBoard::GetXML(bool bSelectedOnly)
{
    int nX0, nY0, nX1, nY1;
    if(bSelectedOnly){
        if(m_SelectState = 1 || m_SelectState = 2){
            // selecting or selected
            return InnGetXML(m_SelectStartLoc.first,
                    m_SelectStartLoc.second, m_SelectStopLoc.first, m_SelectStopLoc.second);
        }else{
            return "<root></root>";
        }
    }else{
        return InnGetXML(0, 0, m_LineV.size() - 1, m_LineV.back().size() - 1);
    }
}

std::string TokenBoard::InnGetXML(int nX0, int nY0, int nX1, int nY1)
{
    std::string szXML;
    if((nY0 > nY1) || (nY0 == nY1 && nX0 > nX1)){ return szXML; }

    szXML = "<root>";
    int nX = nX0;
    int nY = nY0;

    int nLastSection = -1;
    while(!(nX == nX1 && nY == nY1)){
        const auto &rstTB = m_LineV[nY][nX];
        const auto &rstSN = m_SectionV[rstTB.Section];

        if(nLastSection >= 0){
            szXML += "</object>";
        }

        switch(rstSN.Type){
            case SECTIONTYPE_EMOTICON:
                {
                    szXML += "<object type=emoticon set=";
                    szXML += std::to_string(rstSN.Info.Emoticon.Set);
                    szXML += " index=";
                    szXML += std::to_string(rstSN.Info.Emoticon.Index);
                    szXML += ">";
                    break;
                }
            case SECTIONTYPE_EVENTTEXT:
                {
                    if(rstTB.Section != nLastSection){
                        // TODO & TBD
                        // 1. we won't parse ID here
                        // 2. didn't parse style yet
                        // 3. didn't parse color set yet
                        //
                        szXML += "<object type=eventtext font=";
                        szXML += std::to_string(rstSN.Info.Text.Font);
                        szXML += " size=";
                        szXML += std::to_string(rstSN.Info.Text.Size);
                        szXMl += ">";
                    }
                    // truncate to get last 32 bits
                    uint32_t nUTF8Code = rstTB.UTF8CharBox.Cache.Key;
                    szXML += (const char *)(&nUTF8Code);
                    break;
                }
            case SECTIONTYPE_PLAINTEXT:
                {
                    if(rstTB.Section != nLastSection){
                        szXML += "<object type=plaintext font=";
                        szXML += std::to_string(rstSN.Info.Text.Font);
                        szXML += " size=";
                        szXML += std::to_string(rstSN.Info.Text.Size);

                        char szColor[16];
                        std::sprintf(szColor, "0x%08x", rstSN.Info.Text.Color[0]);
                        szXML += " color=";
                        szXML += szColor;
                        szXMl += ">";
                    }
                    // truncate to get last 32 bits
                    uint32_t nUTF8Code = rstTB.UTF8CharBox.Cache.Key;
                    szXML += (const char *)(&nUTF8Code);
                    break;
                }
            default:
                {
                    break;
                }
        }

        nLastSection = rstTB.Section;

        nX++;
        if(nX == m_LineV[nY].size()){
            nX = 0;
            nY++;
        }
    }
}

// Insert a tokenbox at any position (nX, nY) inside the text block, this
// function won't introduce any ``return"
//
// Assume:
//      1. the tokenboard is well-prepared
//      2. we won't allow empty line since don't know how to set height
//
// HowTo:
//      1. if outside the text block, insert at the beginning or append
//         at the very end
//      2. if current line can hold the new box, just insert it
//      3. if can't
//          1. if current line ends with return, create a new line ends
//             with return and contain all needed tokens, insert this 
//             line next to current line. make current line and the new
//             line both end with return
//          2. if not, move token at the end of current line and insert
//             it at the beginning of next line, until current line has
//             enough space to hold the new token
//  return:
//      1. true most likely
//      2. false when current token is tooooooo wide that a whole line
//         even can't hold it
bool TokenBoard::AddTokenBox(TOKENBOX &rstTokenBox, int nX, int nY)
{
    if(m_PW > 0 && m_PW < rstTokenBox.Cache.W){
        // TODO
        // just fail and leave, won't do anything
        return false;
    }

    if(m_LineV.size() == 0){
        AddNewLine({rstTokenBox}, 0);
        m_EndWithReturn.insert(m_EndWithReturn.begin(), m_PW > 0);
        ResetLine(0);
        return true;
    }

    if(nY < 0){
        nX = 0;
        nY = 0;
    }else if(nY >= m_LineV.size()){
        nX = m_LineV.back().size();
        nY = m_LineV.size() - 1;
    }

    nX = std::max(std::min(nX, m_LineV[nY].size()), 0);
    if(m_PW <= 0){
        // we don't have to wrap, easy case
        m_LineV[nY].insert(m_LineV[nY].begin() + nX, rstTokenBox);
        ResetLine(nY);
        return true;
    }

    // we need to wrap the text
    m_LineV[nY].insert(m_LineV[nY].begin() + nX, rstTokenBox);
    int nRawLen = LineRawWidth(nY);

    if(nRawLen + (m_LineV[nY].size() - 1) * m_MinMarginBtwBox <= m_PW){
        // we are good
        // current line have enough space for the new box
        ResetLine(nY);
        return true;
    }

    // bad luck, we need to move some tokens to next line
    if(m_EndWithReturn[nY]){
        // current line ends with return
        // 1. change current line to be ``not ends with return"
        m_EndWithReturn[nY] = false;
        // 2. add a new line, make it end with return
        AddNewLine({m_LineV[nY].back()}, nY + 1);
        m_EndWithReturn.insert(m_EndWithReturn.begin() + nY + 1, true);
        // 3. remove last token from current line
        m_LineV[nY].pop_back();

        // 4. now we have a ``next line" to put all the rest
        //    we need at most two ``next line"s, think about
        //
        //     +----------+        +----------+
        //     | xxxxxxxx |  --->  | xxxxxx   | 
        //     |          |        | +------+ |
        //                         | |      | |
        //                         | +------+ |
        //                         | xx       |
        //
        //
        //    here the inserted token is too large, so we 
        //    need to extra line for it.
        //
        //    but this will handled recursively
    }

    while(m_LineV[nY].size() > 0){
        nRawLen = LineRawWidth(nY);
        if(nRawLen + (m_LineV[nY].size() - 1) * m_MinMarginBtwBox <= m_PW){
            // we are good
            ResetLine(nY);
            return true;
        }else{
            AddTokenBox(nY + 1, 0, m_LineV[nY].back());
            m_LineV[nY].pop_back();
        }
    }

    // this won't be executed
    return false;
}

// Insert a return at any position *inside* a text block
//
// Assumption:
//      1. The current tokenboard is well-prepared
//      2. We won't allow empty line since don't know how to set height
//
// HowTo:
//      1. if outside the text block, do nothing
//      2. if insert return at the beginning of a line, make the last line
//         to be end with return
//      3. if try to insert return at the end of the line
//           1. if current line ends with return, do nothing
//           2. else make current line ends with return
//      4. try to insert return inside a line, if current line ends with
//         return then just add a new line next to current line, and make
//         which ends with return, else
//           1. make current line ends here with return
//           2. insert the rest tokens in front of next line
void TokenBoard::AddReturn(int nX, int nY)
{
    if(m_LineV.size() == 0 || nY < 0 || nY >= m_LineV.size()){
        // outside the text block, do nothing
        return;
    }

    if(nX <= 0){
        // try to insert return at the very beginning
        if(nY > 0){
            // make last line ends with return
            m_EndWithReturn[nY - 1] = true;
            ResetLine(nY - 1);
        }
    }else if(nX >= m_LineV[nY].size()){
        // try to insert return at the end of the line
        if(!m_EndWithReturn[nY]){
            m_EndWithReturn[nY] = true;
            ResetLine(nY);
        }
    }else{
        // try to insert return in the middle
        if(m_EndWithReturn[nY]){
            // current line ends with return
            std::vector<TOKENBOX> stNewLine(m_LineV[nY].begin() + nX, m_LineV[nY].end());
            AddNewLine(nY + 1, stNewLine);
            m_EndWithReturn.insert(m_EndWithReturn.begin() + nY, true);
            ResetLine(nY + 1);
            m_LineV[nY].resize(nX);
            m_EndWithReturn[nY] = true;
            ResetLine(nY);
        }else{
            // it's a long line, process one by one
            int nCount = m_LineV[nY].size() - nX;
            for(int nIndex = 0; nIndex < nCount; ++nIndex){
                AddTokenBox(nY + 1, 0, m_LineV[nY].back());
                m_LineV[nY].pop_back();
            }
            m_EndWithReturn[nY] = true;
            ResetLine(nY);
        }
    }
}

// get the location and shape info of tokenbox
bool TokenBoard::GetTokenBoxInfo(int nX, int nY,
        int *pType, int *pX, int *pY, int *pW, int *pH, int *pW1, int *pW2)
{
    if(nY < 0 || nY >= m_LineV.size()){ return false; }
    if(nX < 0 || nX >= m_LineV[nY].size()){ return false; }

    if(pType){ *pType = m_SectionV[m_LineV[nY][nX].Section].Type; }
    if(pX   ){ *pX    = m_LineV[nY][nX].Cache.StartX;             }
    if(pY   ){ *pY    = m_LineV[nY][nX].Cache.StartY;             }
    if(pW   ){ *pW    = m_LineV[nY][nX].Cache.W;                  }
    if(pH   ){ *pH    = m_LineV[nY][nX].Cache.H;                  }
    if(pW1  ){ *pW1   = m_LineV[nY][nX].State.W1;                 }
    if(pW2  ){ *pW2   = m_LineV[nY][nX].State.W2;                 }

    return true;
}

// remove tokens, for bShadowOnly
//    1:  only shadow will be deleted, for CTRL-X
//    0:  if there is shadow, remove shadow, else remove, current cursor bound tokenbox
void TokenBoard::Delete(bool bShadowOnly)
{
    int nX1, nY1, nX2, nY2;
    if(m_SelectState == 2){
        nX1 = m_SelectStartLoc.first;
        nY1 = m_SelectStartLoc.second;
        nX2 = m_SelectStopLoc.first;
        nY2 = m_SelectStopLoc.second;
        DeleteTokenBox(nX1, nY1, nX2, nY2);
    }else{
        // so when you selecting and hold the left mouse button
        // and then press backspace, it won't be deleted
        if(!bShadowOnly){
            nX1 = nX2 = m_CursorLoc.first;
            nY1 = nY2 = m_CursorLoc.second;
            DeleteTokenBox(nX1, nY1, nX2, nY2);
        }
    }
}
