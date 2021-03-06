
#include "ScriptPCH.h"

/*######
## npc_stormwind_infantry
######*/

enum Northshire
{
    NPC_BLACKROCK_BATTLE_WORG = 49871,
    NPC_STORMWIND_INFANTRY    = 49869,
    NPC_BROTHER_PAXTON        = 951,

    SPELL_SPYING              = 92857,
    SPELL_SNEAKING            = 93046,
    SPELL_SPYGLASS            = 80676,

    SPELL_RENEW               = 93094,
    SPELL_PRAYER_OF_HEALING   = 93091,
    SPELL_FORTITUDE           = 13864,
    SPELL_PENANCE             = 47750,
    SPELL_FLASH_HEAL          = 17843,
};

class npc_stormwind_infantry : public CreatureScript
{
public:
    npc_stormwind_infantry() : CreatureScript("npc_stormwind_infantry") { }

    struct npc_stormwind_infantryAI : public ScriptedAI
    {
        npc_stormwind_infantryAI(Creature* creature) : ScriptedAI(creature)
        {
            me->HandleEmoteCommand(EMOTE_STATE_READY1H);
        }

        EventMap events;

        void EnterCombat(Unit * who)
        {
            me->AddUnitState(UNIT_STATE_ROOT);
            me->HandleEmoteCommand(EMOTE_STATE_READY1H);
        }

        void Reset()
        {
            me->HandleEmoteCommand(EMOTE_STATE_READY1H);
        }

        void DamageDealt(Unit* target, uint32& damage, DamageEffectType /*damageType*/)
        {
            if (target->GetEntry() == NPC_BLACKROCK_BATTLE_WORG)
            {
                damage = 0;
                target->SetHealth(target->GetMaxHealth() * 0.95);
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_stormwind_infantryAI (creature);
    }
};

/*######
## npc_blackrock_battle_worg
######*/

class npc_blackrock_battle_worg : public CreatureScript
{
public:
    npc_blackrock_battle_worg() : CreatureScript("npc_blackrock_battle_worg") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_blackrock_battle_worgAI (creature);
    }

    struct npc_blackrock_battle_worgAI : public ScriptedAI
    {
        npc_blackrock_battle_worgAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void EnterEvadeMode()
        {
            me->RemoveAllAuras();
            me->GetMotionMaster()->MoveTargetedHome();
            me->SetReactState(REACT_PASSIVE);
        }

        void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
        {
            if (target->GetEntry() == NPC_STORMWIND_INFANTRY)
            {
                damage = 0;
                target->SetHealth(target->GetMaxHealth() * 0.95);
            }
        }

        void JustRespawned()
        {
            if (Creature* guard = me->FindNearestCreature(NPC_STORMWIND_INFANTRY, 6.0f, true))
            {
                me->SetReactState(REACT_AGGRESSIVE);
                me->AI()->AttackStart(guard);
            }
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                if (Creature* guard = me->FindNearestCreature(NPC_STORMWIND_INFANTRY, 6.0f, true))
                {
                    guard->getThreatManager().resetAllAggro();
                    guard->CombatStop(true);
                }

                me->SetReactState(REACT_AGGRESSIVE);
                me->AI()->AttackStart(who);
            }
            else
            {
                if (who->isPet())
                {
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->AI()->AttackStart(who);
                }
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                if (Creature* guard = me->FindNearestCreature(NPC_STORMWIND_INFANTRY, 6.0f, true))
                {
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->AI()->AttackStart(guard);
                }
                else
                    return;

            DoMeleeAttackIfReady();
        }
    };
};

enum FearNoEvilStuff
{
    SPELL_RENEWEDLIFE           = 93097,
    SPELL_SPARKLE_VISUAL        = 63048,
    NPC_INJURED_SOLDIER_DUMMY   = 50378,

    ACTION_HEAL                 = 1,
    EVENT_HEALED_1              = 1,
    EVENT_HEALED_2              = 2,
};

class npc_injured_soldier : public CreatureScript
{
public:
    npc_injured_soldier() : CreatureScript("npc_injured_soldier") { }

    struct npc_injured_soldierAI : public ScriptedAI
    {
        npc_injured_soldierAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;
        Unit* player;

        void Reset()
        {
            me->AddAura(SPELL_SPARKLE_VISUAL, me);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_15);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_14);
            me->SetFlag(UNIT_FIELD_BYTES_1, 7);
        }

        void JustRespawned()
        {
            me->AddAura(SPELL_SPARKLE_VISUAL, me);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_15);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_14);
            me->SetFlag(UNIT_FIELD_BYTES_1, 7);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void OnSpellClick(Unit* clicker)
        {
            player = clicker;
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            me->RemoveAurasDueToSpell(SPELL_SPARKLE_VISUAL);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_15);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_14);
            me->RemoveFlag(UNIT_FIELD_BYTES_1, 7);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            events.ScheduleEvent(EVENT_HEALED_1, 2000);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_HEALED_1:
                        Talk(0, player->GetGUID());
                        me->HandleEmoteCommand(EMOTE_ONESHOT_CHEER);
                        player->ToPlayer()->KilledMonsterCredit(me->GetEntry(), NULL);
                        events.ScheduleEvent(EVENT_HEALED_2, 2500);
                        break;
                    case EVENT_HEALED_2:
                        me->GetMotionMaster()->MovePoint(0, -8914.525f, -133.963f, 80.534f, true);
                        me->DespawnOrUnsummon(12000);
                        break;
                    default:
                        break;
                }
            }
        }
    };
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_injured_soldierAI (creature);
    }
};

enum HoggerSpells
{
    SPELL_SUMMON_MINIONS    = 87366,
    SPELL_EVIL_SLICE        = 87337,
    SPELL_EAT               = 87351,
    SPELL_UPSET_STOMACH     = 87352,
    SPELL_TELEPORT_VISUAL   = 41232,
};

enum HoggerNpcs
{
    NPC_HOGGER_MINION       = 46932,
    NPC_GENERAL_MARCUS      = 46942,
    NPC_MAGINOR_DUMAS       = 46940,
    NPC_ANDROMATH           = 46941,
    NPC_RAGAMUFFIN          = 46943,
};

enum HoggerEvents
{
    EVENT_SLICE = 1,
    EVENT_MOVE_EAT,
    EVENT_EAT,
    EVENT_MOVE_CAPTURE,
    EVENT_GRAWL,
    EVENT_TALK_GENERAL_1,
};

enum HoggerTexts
{
    // Hogger
    SAY_HELP        = 1,
    SAY_DONT_HURT   = 2,
    SAY_GRAWL       = 3,
    SAY_NO          = 4,
    // General
    SAY_OUTRO_1     = 1,
    SAY_OUTRO_2     = 2,
    SAY_OUTRO_3     = 3,
};

// The elwynn hogger casts random spells until 50% hp. after that
// he is going to heal himself by eating meat and getting stunned
// after completing this. if the moves to his meat he summons 3 minions
// at 1 hp hogger moves to a point and flees for not getting killed.
// after that 46942 appears with 2 mages 46940(l) and 46941(r)
// after the mismount  2 childrens 46943 appear behind the commander
Position const HoggerPos[] = {-10134.947f, 669.700f, 35.802f, 0.0f};

class npc_hogger : public CreatureScript
{
public:
    npc_hogger() : CreatureScript("npc_hogger") { }

    struct npc_hoggerAI : public ScriptedAI
    {
        npc_hoggerAI(Creature* creature) : ScriptedAI(creature)
        {
            finished = false;
        }

        EventMap events;
        bool finished;

        void Reset()
        {
            events.Reset();
            finished = false;
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_SLICE, 8000);
        }

        void JustDied(Unit* /*killer*/)
        {
        }

        void DamageTaken(Unit* attacker, uint32& damage)
        {
            if (me->HealthBelowPctDamaged(1, damage) && !finished)
            {
                if (damage > me->GetHealth())
                {
                    damage = 0;
                    finished = true;
                    me->SetHealth(1);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetWalk(true);
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    events.Reset();
                    me->GetMotionMaster()->MovePoint(1, HoggerPos->GetPositionX(), HoggerPos->GetPositionY(), HoggerPos->GetPositionZ(), false);
                    Talk(SAY_DONT_HURT);
                }
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            switch (pointId)
            {
                case 1:
                    events.ScheduleEvent(EVENT_MOVE_CAPTURE, 1000);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SLICE:
                        DoCastVictim(SPELL_EVIL_SLICE);
                        events.ScheduleEvent(EVENT_SLICE, 8000);
                        break;
                    case EVENT_MOVE_CAPTURE:
                    {
                        events.ScheduleEvent(EVENT_GRAWL, 3000);
                        std::list<Creature*> units;
                        GetCreatureListWithEntryInGrid(units, me, NPC_GENERAL_MARCUS, 200.0f);
                        for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                        {
                            (*itr)->SetWalk(true);
                            (*itr)->SetPhaseMask(me->GetPhaseMask(), true);
                            (*itr)->CastSpell((*itr), SPELL_TELEPORT_VISUAL);
                            (*itr)->GetMotionMaster()->MovePoint(0, (*itr)->GetPositionX()+cos((*itr)->GetOrientation())*10, (*itr)->GetPositionY()+sin((*itr)->GetOrientation())*10, (*itr)->GetPositionZ(), false);
                        }
                        GetCreatureListWithEntryInGrid(units, me, NPC_ANDROMATH, 200.0f);
                        for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                        {
                            (*itr)->SetWalk(true);
                            (*itr)->SetPhaseMask(me->GetPhaseMask(), true);
                            (*itr)->CastSpell((*itr), SPELL_TELEPORT_VISUAL);
                            (*itr)->GetMotionMaster()->MovePoint(0, (*itr)->GetPositionX()+cos((*itr)->GetOrientation())*10, (*itr)->GetPositionY()+sin((*itr)->GetOrientation())*10, (*itr)->GetPositionZ(), false);
                        }
                        GetCreatureListWithEntryInGrid(units, me, NPC_MAGINOR_DUMAS, 200.0f);
                        for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                        {
                            (*itr)->SetWalk(true);
                            (*itr)->SetPhaseMask(me->GetPhaseMask(), true);
                            (*itr)->CastSpell((*itr), SPELL_TELEPORT_VISUAL);
                            (*itr)->GetMotionMaster()->MovePoint(0, (*itr)->GetPositionX()+cos((*itr)->GetOrientation())*10, (*itr)->GetPositionY()+sin((*itr)->GetOrientation())*10, (*itr)->GetPositionZ(), false);
                        }
                        break;
                    }
                    case EVENT_GRAWL:
                        Talk(SAY_GRAWL);
                        events.ScheduleEvent(EVENT_TALK_GENERAL_1, 3000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

        void EnterEvadeMode()
        {
            me->GetMotionMaster()->MoveTargetedHome();
            me->DespawnCreaturesInArea(NPC_HOGGER_MINION, 200.0f);
        }
    };
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_hoggerAI (creature);
    }
};

void AddSC_elwynn_forest()
{
    new npc_stormwind_infantry();
    new npc_blackrock_battle_worg();
    new npc_injured_soldier();
    new npc_hogger();
}