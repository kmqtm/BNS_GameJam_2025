#include "EnemyDataSystem.h"

using namespace s3d;

namespace
{
	// 安全な取り出しヘルパー
	double GetDoubleOr(const JSON& obj, StringView key, double def)
	{
		return (obj.hasElement(key) ? obj[key].get<double>() : def);
	}

	bool GetBoolOr(const JSON& obj, StringView key, bool def)
	{
		return (obj.hasElement(key) ? obj[key].get<bool>() : def);
	}

	String GetStringOr(const JSON& obj, StringView key, const String& def)
	{
		return (obj.hasElement(key) ? obj[key].getString() : def);
	}
}

const JSON EnemyDataSystem::kEmptyJSON = JSON{};

EnemyDataSystem& EnemyDataSystem::GetInstance()
{
	static EnemyDataSystem instance;
	return instance;
}

EnemyDataSystem::EnemyDataSystem()
{
	LoadEnemyData();
}

void EnemyDataSystem::LoadEnemyData()
{
	enemy_data_ = JSON::Load(kEnemyDataPath);

	if(not enemy_data_)
	{
		Print << U"エラー: EnemyData.json の読み込みに失敗 → {}"_fmt(kEnemyDataPath);
		return;
	}
	if(not enemy_data_.hasElement(U"enemies") || not enemy_data_[U"enemies"].isObject())
	{
		Print << U"エラー: EnemyData.json に 'enemies' オブジェクトがありません";
		enemy_data_ = JSON{};
		return;
	}

	//Print << U"EnemyDataManager: {} を読み込みました"_fmt(kEnemyDataPath);
}

const JSON& EnemyDataSystem::GetData(const String& type) const
{
	// JSONが正常にロードされていない場合
	if(not enemy_data_)
	{
		return kEmptyJSON;
	}

	// "enemies" キーからデータを取得
	const auto& enemies = enemy_data_[U"enemies"];

	// 指定されたタイプが存在するか確認
	if(not enemies.hasElement(type))
	{
		Print << U"警告: 敵タイプ '{}' のデータが見つかりません"_fmt(type);
		return kEmptyJSON;
	}

	return enemies[type];
}

bool EnemyDataSystem::IsLoaded() const
{
	return enemy_data_.isObject() && enemy_data_.hasElement(U"enemies");
}

EnemyDataSystem::BehaviorKind EnemyDataSystem::ToBehaviorKind(const String& s)
{
	if(s == U"Patrol") return BehaviorKind::Patrol;
	if(s == U"BackAndForth") return BehaviorKind::BackAndForth;
	return BehaviorKind::Stationary;
}

EnemyDataSystem::ColliderKind EnemyDataSystem::ToColliderKind(const String& s)
{
	if(s == U"Circle") return ColliderKind::Circle;
	return ColliderKind::RectF;
}

Optional<EnemyDataSystem::AnimationSpec> EnemyDataSystem::ReadAnimationSpec(const JSON& anim)
{
	if(not anim.isObject()) return none;

	AnimationSpec spec;

	if(anim.hasElement(U"textures") && anim[U"textures"].isArray())
	{
		for(const auto& t : anim[U"textures"].arrayView())
		{
			spec.textures << t.getString();
		}
	}

	if(spec.textures.isEmpty())
	{
		Print << U"警告: textures配列が空か、存在しないアニメーション定義があります．";
		return none;
	}

	spec.frame_duration_sec = GetDoubleOr(anim, U"frame_duration", 0.5);
	spec.is_looping = GetBoolOr(anim, U"is_looping", true);

	return spec;
}

Optional<EnemyDataSystem::EnemySpec> EnemyDataSystem::TryGetSpec(const String& type) const
{
	if(not IsLoaded()) return none;

	const auto& enemies = enemy_data_[U"enemies"];
	if(not enemies.hasElement(type) || not enemies[type].isObject())
	{
		return none;
	}

	const auto& src = enemies[type];

	EnemySpec spec;

	// behavior
	spec.behavior = ToBehaviorKind(GetStringOr(src, U"behavior", U"Stationary"));

	// physics_size
	if(src.hasElement(U"physics_size") && src[U"physics_size"].isObject())
	{
		const auto& ps = src[U"physics_size"];
		spec.physics_size.x = static_cast<int32>(GetDoubleOr(ps, U"width", 0.0));
		spec.physics_size.y = static_cast<int32>(GetDoubleOr(ps, U"height", 0.0));
	}

	// collider
	spec.collider_shape = ToColliderKind(GetStringOr(src, U"collider_shape", U"RectF"));
	if(src.hasElement(U"collider_size") && src[U"collider_size"].isObject())
	{
		const auto& cs = src[U"collider_size"];
		if(spec.collider_shape == ColliderKind::RectF)
		{
			spec.collider_width = GetDoubleOr(cs, U"width", 0.0);
			spec.collider_height = GetDoubleOr(cs, U"height", 0.0);
		}
		else
		{
			spec.collider_radius = GetDoubleOr(cs, U"radius", 0.0);
		}
	}

	// movement params
	spec.speed = GetDoubleOr(src, U"speed", 0.0);
	spec.collision_offset = GetDoubleOr(src, U"collision_offset", 0.0);
	spec.max_travel_distance = GetDoubleOr(src, U"max_travel_distance", 0.0);
	spec.initial_facing_right = GetBoolOr(src, U"initial_facing_right", false);
	spec.initial_velocity_x = GetDoubleOr(src, U"initial_velocity_x", 0.0);

	// animations: "move" / "idle" のみ安全に取り込む
	if(src.hasElement(U"animations") && src[U"animations"].isObject())
	{
		const auto& anims = src[U"animations"];

		if(anims.hasElement(U"move"))
		{
			if(auto a = ReadAnimationSpec(anims[U"move"]))
			{
				spec.animations.emplace(U"move", *a);
			}
		}
		if(anims.hasElement(U"idle"))
		{
			if(auto a = ReadAnimationSpec(anims[U"idle"]))
			{
				spec.animations.emplace(U"idle", *a);
			}
		}
	}

	return spec;
}
