
#include "dragonsoul.h"
#include "Vehicle.h"
#include "Unit.h"

enum Spells
{
    // Morchok
    SPELL_SIEGE_MISSILE                 = 110307,
    SPELL_CRUSH_ARMOR                   = 103687,
    SPELL_STOMP                         = 103414,
    SPELL_SUMMON_RESONATING_CRYSTAL     = 103640,
    SPELL_EARTHEN_VORTEX                = 103821,
    SPELL_EARTHEN_VORTEX_CONTROL_VEHICLE = 109615,
    SPELL_FALLING_FRAGMENTS             = 103176,
    SPELL_FALLING_FRAGMENT              = 103177,
    SPELL_FALLING_FRAGMENT_TRIGGERED    = 103178,
    SPELL_BLACK_BLOOD_OF_THE_EARTH_1    = 103851,
    SPELL_BLACK_BLOOD_OF_THE_EARTH_2    = 103785,

    // Resonating Crystal
    SPELL_RESONATING_CRYSTAL_AURA       = 103494,
    SPELL_RESONATING_CRYSTAL_EXPLOSION  = 103545,
    SPELL_SAFE                          = 103541,
    SPELL_WARNING                       = 103536,
    SPELL_DANGER                        = 103534,
    SPELL_TARGET_SELECTION              = 103528,
    SPELL_KNOCKBACK_SELF_VERTICAL       = 103673,
    SPELL_FALLING_FRAGMENT_KNOCKBACK    = 103174,

    SPELL_BLOOD_VISUAL                  = 103180,
    SPELL_FURIOUS                       = 103846,
};

enum Texts
{
    SAY_SIEGE_1                     = 0,
    SAY_SIEGE_2                     = 1,
    SAY_SIEGE_3                     = 2,
    SAY_SIEGE_4                     = 3,
    SAY_AGGRO                       = 4,
    SAY_SLAY                        = 5,
    SAY_RESONATING_CRYSTAL          = 6,
    SAY_ANNOUNCE_RESONATING_CRYSTAL = 7,
    SAY_EARTHEN_VORTEX              = 8,
    SAY_BLOOD_OF_THE_EARTH          = 9,
    SAY_DEATH                       = 10,
};

enum Phases
{
    PHASE_INTRO     = 1,
    PHASE_COMBAT    = 2,
};

enum Events
{
    // Morchok
    EVENT_SIEGE_INTRO = 1,
    EVENT_CRUSH_ARMOR,
    EVENT_STOMP,
    EVENT_RESONATING_CRYSTAL,
    EVENT_EARTHEN_VORTEX,
    EVENT_BLACK_BLOOD_OF_THE_EARTH,

    // Resonating Crystal
    EVENT_EXPLODE,

    // Earthen Vortex
    EVENT_DESPAWN,
};

enum Actions
{
    ACTION_TRIGGER_INTRO_1 = 1,
    ACTION_TRIGGER_INTRO_2,
    ACTION_TRIGGER_INTRO_3,
    ACTION_TRIGGER_INTRO_4,
};

enum Points
{
    POINT_MORCHOK = 1,
};

Position const SiegePos1 = {-1874.589f, -2366.159f, 128.197f, 0.0f};
Position const SiegePos2 = {-1861.810f, -2406.899f, 143.921f, 0.0f};
Position const SiegePos3 = {-1869.317f, -2428.761f, 141.404f, 0.0f};

class boss_morchok : public CreatureScript
{
public:
    boss_morchok() : CreatureScript("boss_morchok") { }

    struct boss_morchokAI : public BossAI
    {
        boss_morchokAI(Creature* creature) : BossAI(creature, DATA_MORCHOK)
        {
            _furious = false;
        }

        bool _furious;

        void Reset()
        {
            _Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            Talk(SAY_AGGRO);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            events.SetPhase(PHASE_COMBAT);
            events.ScheduleEvent(EVENT_CRUSH_ARMOR, 6000, 0, PHASE_COMBAT);
            events.ScheduleEvent(EVENT_STOMP, 12500, 0, PHASE_COMBAT);
            events.ScheduleEvent(EVENT_RESONATING_CRYSTAL, 18000, 0, PHASE_COMBAT);
            events.ScheduleEvent(EVENT_EARTHEN_VORTEX, 56000, 0, PHASE_COMBAT);
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            Talk(SAY_DEATH);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SAFE);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WARNING);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_DANGER);
        }

        void EnterEvadeMode()
        {
            _EnterEvadeMode();
            events.Reset();
            _furious = false;
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            summons.DespawnAll();
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SAFE);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WARNING);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_DANGER);
            _DespawnAtEvade();
            me->GetMotionMaster()->MoveTargetedHome();
        }

        void KilledUnit(Unit* killed)
        {
            if (killed->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_SLAY);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            switch (summon->GetEntry())
            {
                case 0:
                    break;
                default:
                    break;
            }
        }

        void DoAction(int32 action)
        {
            switch (action)
            {
                case ACTION_TRIGGER_INTRO_1:
                    TalkToMap(SAY_SIEGE_1);
                    break;
                case ACTION_TRIGGER_INTRO_2:
                    TalkToMap(SAY_SIEGE_2);
                    break;
                case ACTION_TRIGGER_INTRO_3:
                    TalkToMap(SAY_SIEGE_3);
                    break;
                case ACTION_TRIGGER_INTRO_4:
                    TalkToMap(SAY_SIEGE_4);
                    break;
                default:
                    break;
            }
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage)
        {
            if (HealthBelowPct(21) && !_furious)
            {
                DoCast(me, SPELL_FURIOUS, true);
                _furious = true;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!(events.IsInPhase(PHASE_INTRO)))
                if (!UpdateVictim())
                    return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CRUSH_ARMOR:
                        me->PlaySpellVisual(21700, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                        DoCastVictim(SPELL_CRUSH_ARMOR);
                        events.ScheduleEvent(EVENT_CRUSH_ARMOR, 8000, 0, PHASE_COMBAT);
                        break;
                    case EVENT_STOMP:
                        DoCast(me, SPELL_STOMP);
                        events.ScheduleEvent(EVENT_STOMP, 11500, 0, PHASE_COMBAT);
                        break;
                    case EVENT_RESONATING_CRYSTAL:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true, 0))
                            DoCast(target, SPELL_SUMMON_RESONATING_CRYSTAL);
                        Talk(SAY_RESONATING_CRYSTAL);
                        Talk(SAY_ANNOUNCE_RESONATING_CRYSTAL);
                        events.ScheduleEvent(EVENT_RESONATING_CRYSTAL, 14500, 0, PHASE_COMBAT);
                        break;
                    case EVENT_EARTHEN_VORTEX:
                        DoCast(me, SPELL_EARTHEN_VORTEX);
                        DoCast(me, SPELL_FALLING_FRAGMENTS);
                        Talk(SAY_EARTHEN_VORTEX);
                        events.ScheduleEvent(EVENT_BLACK_BLOOD_OF_THE_EARTH, 4500);
                        break;
                    case EVENT_BLACK_BLOOD_OF_THE_EARTH:
                        Talk(SAY_BLOOD_OF_THE_EARTH);
                        DoCast(me, SPELL_BLACK_BLOOD_OF_THE_EARTH_1);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_morchokAI(creature);
    }
};

class npc_ds_resonating_crystal : public CreatureScript
{
public:
    npc_ds_resonating_crystal() : CreatureScript("npc_ds_resonating_crystal") { }

    struct npc_ds_resonating_crystalAI : public ScriptedAI
    {
        npc_ds_resonating_crystalAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void IsSummonedBy(Unit* /*summoner*/)
        {
            me->SetInCombatWithZone();
            DoCast(me, SPELL_RESONATING_CRYSTAL_AURA, true);
            events.ScheduleEvent(EVENT_EXPLODE, 12000);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_EXPLODE:
                    DoCast(me, SPELL_KNOCKBACK_SELF_VERTICAL);
                    DoCast(me, SPELL_RESONATING_CRYSTAL_EXPLOSION);
                    me->RemoveAurasDueToSpell(SPELL_RESONATING_CRYSTAL_AURA);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SAFE);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WARNING);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_DANGER);
                    me->DespawnOrUnsummon(2000);
                    break;
                default:
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ds_resonating_crystalAI(creature);
    }
};

class npc_ds_earthen_vortex : public CreatureScript
{
public:
    npc_ds_earthen_vortex() : CreatureScript("npc_ds_earthen_vortex") { }

    struct npc_ds_earthen_vortexAI : public ScriptedAI
    {
        npc_ds_earthen_vortexAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        EventMap events;


        void OnCharmed(bool apply)
        {
            me->NeedChangeAI = false;
            me->IsAIEnabled = true;
        }

        void IsSummonedBy(Unit* summoner)
        {
            //summoner->RemoveUnitMovementFlag(MOVEMENTFLAG_MASK_MOVING);
            //summoner->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
            summoner->CastSpell(me, SPELL_EARTHEN_VORTEX_CONTROL_VEHICLE, true);
            me->MonsterYell("Willkommen in den fliegenden Flitzepuff. Bitte schnallen sie sich und ihre Hosenschlange ���n.", LANG_UNIVERSAL, 0);
            if (Creature* morchok = me->FindNearestCreature(BOSS_MORCHOK, 200.0f, true))
                me->GetMotionMaster()->MovePoint(POINT_MORCHOK, morchok->GetPositionX(), morchok->GetPositionY(), morchok->GetPositionZ() + 1.0f, true);
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE)
                return;

            switch (pointId)
            {
                case POINT_MORCHOK:
                    events.ScheduleEvent(EVENT_DESPAWN, 1);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_DESPAWN:
                        me->DespawnOrUnsummon();
                        break;
                    default:
                        break;
                }
            }

        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ds_earthen_vortexAI(creature);
    }
};

class spell_ds_target_selection : public SpellScriptLoader
{
public:
    spell_ds_target_selection() : SpellScriptLoader("spell_ds_target_selection") { }

    class spell_ds_target_selection_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ds_target_selection_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (targets.empty())
                return;

            if (Unit* caster = GetCaster())
                if (uint8 targetSize = caster->GetMap()->Is25ManRaid() ? 7 : 3)
                    if (targets.size() > targetSize)
                    {
                        targets.sort(Trinity::ObjectDistanceOrderPred(caster));
                        targets.resize(targetSize);
                    }
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
                if (Unit* target = GetHitUnit())
                {
                    if (caster->GetDistance2d(target) <= caster->GetFloatValue(UNIT_FIELD_COMBATREACH))
                    {
                        caster->CastSpell(target, SPELL_SAFE, true);
                        target->RemoveAurasDueToSpell(SPELL_WARNING);
                        target->RemoveAurasDueToSpell(SPELL_DANGER);
                    }
                    else if (caster->GetDistance2d(target) <= 15.0f)
                    {
                        caster->CastSpell(target, SPELL_WARNING, true);
                        target->RemoveAurasDueToSpell(SPELL_SAFE);
                        target->RemoveAurasDueToSpell(SPELL_DANGER);
                    }
                    else
                    {
                        caster->CastSpell(target, SPELL_DANGER, true);
                        target->RemoveAurasDueToSpell(SPELL_SAFE);
                        target->RemoveAurasDueToSpell(SPELL_WARNING);
                    }
                }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_ds_target_selection_SpellScript::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ds_target_selection_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_ds_target_selection_SpellScript();
    }
};

class spell_ds_resonating_crystal_explosion : public SpellScriptLoader
{
public:
    spell_ds_resonating_crystal_explosion() : SpellScriptLoader("spell_ds_resonating_crystal_explosion") { }

    class spell_ds_resonating_crystal_explosion_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ds_resonating_crystal_explosion_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (targets.empty())
                return;

            if (Unit* caster = GetCaster())
                if (uint8 targetSize = caster->GetMap()->Is25ManRaid() ? 7 : 3)
                    if (targets.size() > targetSize)
                    {
                        targets.sort(Trinity::ObjectDistanceOrderPred(caster));
                        targets.resize(targetSize);
                    }
        }

        void CalculateDamage(SpellEffIndex /*effIndex*/)
        {
            if (Unit* target = GetHitUnit())
                if (Unit* caster = GetCaster())
                {
                    float distance = GetCaster()->GetDistance2d(GetHitUnit());
                    uint64 damage = GetEffectValue();
                    uint64 newDamage = 0;

                    if (target->HasAura(SPELL_WARNING) || target->HasAura(SPELL_DANGER))
                        if (distance > 1.0f)
                        {
                            newDamage = damage + (distance * 2000);
                            SetHitDamage(newDamage);
                        }
                }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_ds_resonating_crystal_explosion_SpellScript::CalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ds_resonating_crystal_explosion_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ds_resonating_crystal_explosion_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ds_resonating_crystal_explosion_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_DEST_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_ds_resonating_crystal_explosion_SpellScript();
    }
};

class spell_ds_stomp : public SpellScriptLoader
{
public:
    spell_ds_stomp() : SpellScriptLoader("spell_ds_stomp") { }

    class spell_ds_stomp_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ds_stomp_SpellScript);

        bool Load()
        {
            _targetCount = 0;
            return true;
        }

        void CountTargets(std::list<WorldObject*>& targets)
        {
            _targetCount = targets.size();
        }

        void SplitDamage()
        {
            if (!_targetCount)
                return;

            uint64 basicDamage = GetHitDamage();
            uint64 newDamage = 0;

            if (Unit* target = GetHitUnit())
                if (Unit* caster = GetCaster())
                    if (Unit* victim = caster->getVictim())
                    {
                        newDamage = basicDamage / _targetCount;

                        if (target == victim)
                            newDamage = (basicDamage / _targetCount) * 2;

                        if (Unit* ally = victim->FindNearestPlayer(50.0f, true))
                            if (target == ally)
                                newDamage = (basicDamage / _targetCount) * 2;

                        SetHitDamage(newDamage);
                    }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ds_stomp_SpellScript::CountTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_104);
            OnHit += SpellHitFn(spell_ds_stomp_SpellScript::SplitDamage);
        }

    private:
        uint32 _targetCount;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_ds_stomp_SpellScript();
    }
};

class spell_ds_falling_fragments : public SpellScriptLoader
{
public:
    spell_ds_falling_fragments() : SpellScriptLoader("spell_ds_falling_fragments") { }

    class spell_ds_falling_fragments_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_ds_falling_fragments_AuraScript);

        void OnPeriodic(AuraEffect const* /*aurEff*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (float angle = frand(0.0f, (2 * M_PI)))
                    if (float dist = frand(caster->GetFloatValue(UNIT_FIELD_COMBATREACH), 40.0f))
                    {
                        float x = caster->GetPositionX() + cos(angle) * dist;
                        float y = caster->GetPositionY() + sin(angle) * dist;
                        float z = caster->GetPositionZ() + 20.0f;
                        float ground = caster->GetMap()->GetWaterOrGroundLevel(x, y, z, &ground);
                        caster->CastSpell(x, y, ground, SPELL_FALLING_FRAGMENT, true);
                    }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_ds_falling_fragments_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_ds_falling_fragments_AuraScript();
    }
};

class spell_ds_black_blood_of_the_earth_damage : public SpellScriptLoader
{
public:
    spell_ds_black_blood_of_the_earth_damage() : SpellScriptLoader("spell_ds_black_blood_of_the_earth_damage") { }

    class spell_ds_black_blood_of_the_earth_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ds_black_blood_of_the_earth_damage_SpellScript);

        class BehindStalaktiteSelector
        {
        public:
            BehindStalaktiteSelector(WorldObject* unit) : caster(unit) { }

            bool operator() (WorldObject* unit)
            {
                if (Unit* target = unit->ToUnit())
                {
                    std::list<GameObject*> blockList;
                    caster->GetGameObjectListWithEntryInGrid(blockList, GO_INNER_WALL, 300.0f);
                    if (!blockList.empty())
                    {
                        for (std::list<GameObject*>::const_iterator itr = blockList.begin(); itr != blockList.end(); ++itr)
                        {
                            if (!(*itr)->IsInvisibleDueToDespawn())
                            {
                                if ((*itr)->IsInBetween(caster, target, 3.0f))
                                    return true;
                            }
                        }
                    }
                }
                return false;
            }

        private:
            WorldObject* caster;
        };

        class RadiusCheckFilter
        {
        public:
            RadiusCheckFilter(Unit* caster) : caster(caster) { }

            bool operator()(WorldObject* object)
            {
                if (Aura* blood = caster->GetAura(SPELL_BLACK_BLOOD_OF_THE_EARTH_1))
                    if (uint32 stack = blood->GetEffect(EFFECT_0)->GetTickNumber())
                        if (float radius = 5.0f * stack)
                            return (object->GetDistance2d(caster->GetPositionX(), caster->GetPositionY()) > radius);
                return false;
            }

        private:
            Unit* caster;
        };

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            if (Unit* caster = GetCaster())
            {
                unitList.remove_if(RadiusCheckFilter(caster));

                if (!unitList.empty())
                    return;

                unitList.remove_if(BehindStalaktiteSelector(caster));
            }
            else
                unitList.clear();
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ds_black_blood_of_the_earth_damage_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ds_black_blood_of_the_earth_damage_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_ds_black_blood_of_the_earth_damage_SpellScript();
    }
};

class spell_ds_black_blood_of_the_earth_aura : public SpellScriptLoader
{
public:
    spell_ds_black_blood_of_the_earth_aura() : SpellScriptLoader("spell_ds_black_blood_of_the_earth_aura") { }

    class spell_ds_black_blood_of_the_earth_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_ds_black_blood_of_the_earth_aura_AuraScript);

        void OnPeriodic(AuraEffect const* aurEff)
        {
            if (Unit* caster = GetCaster())
            {
                /*
                if (uint32 stack = GetEffect(EFFECT_0)->GetTickNumber())
                    if (stack > 1)
                    {
                        if (uint32 numberPerCircle = stack * 2.5f)
                            if (float radius = 5.0f * stack)
                            {
                                for (uint32 i = 0; i <= numberPerCircle; i++)
                                {
                                    float angle = ((2 * M_PI) / numberPerCircle) * i;
                                    float x = caster->GetPositionX() + cos(angle) * radius;
                                    float y = caster->GetPositionZ() + sin(angle) * radius;
                                    float z = caster->GetPositionZ();
                                    caster->CastSpell(x, y, z, SPELL_BLOOD_VISUAL, true);
                                }
                            }
                    }
                */
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_ds_black_blood_of_the_earth_aura_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_ds_black_blood_of_the_earth_aura_AuraScript();
    }
};

void AddSC_boss_morchok()
{
    new boss_morchok();
    new npc_ds_resonating_crystal();
    new npc_ds_earthen_vortex();
    new spell_ds_target_selection();
    new spell_ds_resonating_crystal_explosion();
    new spell_ds_stomp();
    new spell_ds_falling_fragments();
    new spell_ds_black_blood_of_the_earth_damage();
    new spell_ds_black_blood_of_the_earth_aura();
}
