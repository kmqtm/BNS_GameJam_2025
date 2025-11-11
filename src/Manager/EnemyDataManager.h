#pragma once

#include <Siv3D.hpp>

// AssetController と同様の「集中管理＋防御的読み込み」方針
class EnemyDataManager
{
public:
	static EnemyDataManager& GetInstance();

	// JSON 全体が有効にロードされているか
	bool IsLoaded() const;

	// 型安全な仕様データ
	enum class BehaviorKind { Stationary, Patrol, BackAndForth };
	enum class ColliderKind { RectF, Circle };

	struct AnimationSpec
	{
		s3d::Array<s3d::String> textures;
		double frame_duration_sec = 0.5;
		bool is_looping = true;
	};

	struct EnemySpec
	{
		BehaviorKind behavior = BehaviorKind::Stationary;

		s3d::Size physics_size{ 0, 0 };

		ColliderKind collider_shape = ColliderKind::RectF;
		// RectF 用
		double collider_width = 0.0;
		double collider_height = 0.0;
		// Circle 用
		double collider_radius = 0.0;

		// Patrol / BackAndForth 共通・専用
		double speed = 0.0;                 // Patrol / BackAndForth
		double collision_offset = 0.0;      // Patrol
		double max_travel_distance = 0.0;   // BackAndForth
		bool initial_facing_right = false;  // Patrol
		double initial_velocity_x = 0.0;    // BackAndForth

		// 必要なアニメーション（"move", "idle" を想定）
		s3d::HashTable<s3d::String, AnimationSpec> animations;
	};

	// 指定タイプの仕様を安全に取得（存在しなければ none）
	s3d::Optional<EnemySpec> TryGetSpec(const s3d::String& type) const;

	// 指定タイプの JSON データを取得（存在しなければ空の JSON を返す）
	const s3d::JSON& GetData(const s3d::String& type) const;

	EnemyDataManager(const EnemyDataManager&) = delete;
	EnemyDataManager& operator=(const EnemyDataManager&) = delete;

private:
	EnemyDataManager();
	void LoadEnemyData();

	// ヘルパー
	static BehaviorKind ToBehaviorKind(const s3d::String& s);
	static ColliderKind ToColliderKind(const s3d::String& s);
	static s3d::Optional<AnimationSpec> ReadAnimationSpec(const s3d::JSON& anim);

	// JSON root
	s3d::JSON enemy_data_;
	static const s3d::JSON kEmptyJSON;

	// パスは App 配下を基準に "asset/..." で OK（ログにも同パスが出ていました）
	static constexpr s3d::FilePathView kEnemyDataPath = U"asset/EnemyData.json";
};
