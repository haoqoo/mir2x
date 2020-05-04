-- =====================================================================================
--
--       Filename: default.lua
--        Created: 08/31/2015 08:52:57 PM
--    Description: lua 5.3
--
--        Version: 1.0
--       Revision: none
--       Compiler: lua
--
--         Author: ANHONG
--          Email: anhonghe@gmail.com
--   Organization: USTC
--
-- =====================================================================================

-- NPC script
-- provides the table: processNPCEvent for event processing

addLog(LOGTYPE_INFO, string.format('NPC %s sources default.lua', getName()))
processNPCEvent =
{
    ["npc_init"] = function(uid, value)
        sayXML(uid, string.format(
        [[
            <layout>                                                                
                <par>客官%s你好，我是%s，欢迎来到传奇旧时光！<emoji id="0"/></par>
                <par>有什么可以为你效劳的吗？</par>                                 
                <par></par>                                                         
                <par><event id="event_1">如何快速升级</event></par>                       
                <par><event id="close">关闭</event></par>                           
            </layout>                                                               
        ]], getUIDString(uid), getName()))
    end,

    ["event_1"] = function(uid, value)
        sayXML(uid,
        [[
            <layout>                                        
                <par>多多上线打怪升级！<emoji id="1"/></par>
                <par><event id="close">关闭</event></par>   
            </layout>                                       
        ]])
    end,
}