/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
SDName: Teldrassil
SD%Complete: 100
SDComment: Quest support: 938
SDCategory: Teldrassil
EndScriptData */

/* ContentData
npc_mist
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedFollowerAI.h"
#include "Player.h"

/*####
# npc_mist
####*/

enum Mist
{
    SAY_AT_HOME             = 0,
    EMOTE_AT_HOME           = 1,
    QUEST_MIST              = 938,
    NPC_ARYNIA              = 3519,
    FACTION_DARNASSUS       = 79
};

class npc_mist : public CreatureScript
{
public:
    npc_mist() : CreatureScript("npc_mist") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_MIST)
            if (npc_mistAI* pMistAI = CAST_AI(npc_mist::npc_mistAI, creature->AI()))
                pMistAI->StartFollow(player, FACTION_DARNASSUS, quest);

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_mistAI(creature);
    }

    struct npc_mistAI : public FollowerAI
    {
        npc_mistAI(Creature* creature) : FollowerAI(creature) { }

        void Reset() { }

        void MoveInLineOfSight(Unit* who)
        {
            FollowerAI::MoveInLineOfSight(who);

            if (!me->getVictim() && !HasFollowState(STATE_FOLLOW_COMPLETE) && who->GetEntry() == NPC_ARYNIA)
            {
                if (me->IsWithinDistInMap(who, 10.0f))
                {
                    Talk(SAY_AT_HOME, who->GetGUID());
                    DoComplete();
                }
            }
        }

        void DoComplete()
        {
            Talk(EMOTE_AT_HOME);

            Player* player = GetLeaderForFollower();
            if (player && player->GetQuestStatus(QUEST_MIST) == QUEST_STATUS_INCOMPLETE)
                player->GroupEventHappens(QUEST_MIST, me);

            //The follow is over (and for later development, run off to the woods before really end)
            SetFollowComplete();
        }

        //call not needed here, no known abilities
        /*void UpdateFollowerAI(uint32 Diff)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }*/
    };

};

class npc_wounded_sentinel : public CreatureScript
{
public:
    npc_wounded_sentinel() : CreatureScript("npc_wounded_sentinel") { }

    struct npc_wounded_sentinelAI : public ScriptedAI
    {
        npc_wounded_sentinelAI(Creature* creature) : ScriptedAI(creature) {}

        enum Spells
        {
            SPELL_REJUVENATION  = 774
        };

        enum Quests
        {
            QUEST_REJUVENATING_TOUCH    = 26948,
            QUEST_HEALING_THE_WOUNDED   = 26949
        };

        enum Credits
        {
            QUEST_CREDIT_WOUNDED_SENTINEL = 44175
        };

        void SpellHit(Unit* caster, SpellInfo const* spell)
        {
            if (caster->GetTypeId() == TYPEID_PLAYER)
            {
                switch (spell->Id)
                {
                    case 774: // Rejuvenation
                    {
                        if (caster->ToPlayer()->GetQuestStatus(QUEST_REJUVENATING_TOUCH) == QUEST_STATUS_INCOMPLETE)
                            caster->ToPlayer()->KilledMonsterCredit(QUEST_CREDIT_WOUNDED_SENTINEL);
                        break;
                    }
                    case 2061: // Flash Heal
                    {
                        if (caster->ToPlayer()->GetQuestStatus(QUEST_HEALING_THE_WOUNDED) == QUEST_STATUS_INCOMPLETE)
                            caster->ToPlayer()->KilledMonsterCredit(QUEST_CREDIT_WOUNDED_SENTINEL);
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_wounded_sentinelAI (creature);
    }
};

class npc_grellkin : public CreatureScript
{
public:
    npc_grellkin() : CreatureScript("npc_grellkin") { }

    struct npc_grellkinAI : public ScriptedAI
    {
        npc_grellkinAI(Creature* creature) : ScriptedAI(creature) {}

        enum Id
        {
            SPELL_NATURE_FURY               = 65455,
            QUEST_NATURE_REPRISAL           = 13946,
            QUEST_CREDIT_GRELLKIN           = 34440
        };

        void JustDied(Unit* victim)
        {
            if (!victim)
                return;

            if (victim->GetTypeId() != TYPEID_PLAYER)
                return;

            if (me->HasAura(SPELL_NATURE_FURY, victim->GetGUID()))
                victim->ToPlayer()->KilledMonsterCredit(QUEST_CREDIT_GRELLKIN);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_grellkinAI (creature);
    }
};

void AddSC_teldrassil()
{
    new npc_mist();
    new npc_wounded_sentinel();
    new npc_grellkin();
}
