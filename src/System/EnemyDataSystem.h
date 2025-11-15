#pragma once

/// @file EnemyDataSystem.h
/// @brief 敵データ管理システムの定義

#include <Siv3D.hpp>

/// @brief 敵の仕様データを一元管理するシングルトン
///
/// JSONファイルから敵の行動，物理属性，アニメーションなどのデータを読み込み，
/// 安全にアクセスできるインターフェースを提供する
class EnemyDataSystem
{
public:
	/// @brief シングルトンインスタンスを取得する
	/// @return EnemyDataSystemのインスタンス
	static EnemyDataSystem& GetInstance();

	/// @brief JSON全体が有効にロードされているか
	/// @return ロード完了している場合はtrue
	bool IsLoaded() const;

	/// @brief 敵の行動パターン
	enum class BehaviorKind { Stationary, Patrol, BackAndForth };

	/// @brief コライダー図形の種類
	enum class ColliderKind { RectF, Circle };

	/// @brief アニメーション仕様
	struct AnimationSpec
	{
		s3d::Array<s3d::String> textures;           ///< テクスチャアセット名のリスト
		double frame_duration_sec = 0.5;            ///< フレーム表示時間(秒)
		bool is_looping = true;                     ///< ループ再生するか
	};

	/// @brief 敵の仕様
	struct EnemySpec
	{
		BehaviorKind behavior = BehaviorKind::Stationary;  ///< 行動パターン

		s3d::Size physics_size{ 0, 0 };                     ///< 物理サイズ

		ColliderKind collider_shape = ColliderKind::RectF;  ///< コライダー図形
		double collider_width = 0.0;                        ///< コライダー幅(RectF用)
		double collider_height = 0.0;                       ///< コライダー高さ(RectF用)
		double collider_radius = 0.0;                       ///< コライダー半径(Circle用)

		double speed = 0.0;                         ///< 移動速度(Patrol/BackAndForth用)
		double collision_offset = 0.0;              ///< 衝突判定オフセット(Patrol用)
		double max_travel_distance = 0.0;           ///< 最大移動距離(BackAndForth用)
		bool initial_facing_right = false;          ///< 初期向き(Patrol用)
		double initial_velocity_x = 0.0;            ///< 初期速度X(BackAndForth用)

		/// @brief アニメーション("move","idle"など)
		s3d::HashTable<s3d::String, AnimationSpec> animations;
	};

	/// @brief 指定したタイプの敵仕様を取得する
	/// @param type 敵タイプ
	/// @return 敵仕様。存在しない場合はnullopt
	s3d::Optional<EnemySpec> TryGetSpec(const s3d::String& type) const;

	/// @brief 指定したタイプのJSONデータを取得する
	/// @param type 敵タイプ
	/// @return JSONデータ。存在しない場合は空のJSON
	const s3d::JSON& GetData(const s3d::String& type) const;

	/// @brief コピーコンストラクタ(削除)
	EnemyDataSystem(const EnemyDataSystem&) = delete;

	/// @brief コピー代入演算子(削除)
	EnemyDataSystem& operator=(const EnemyDataSystem&) = delete;

private:
	/// @brief コンストラクタ
	EnemyDataSystem();

	/// @brief 敵データをロードする
	void LoadEnemyData();

	/// @brief 文字列を行動パターンに変換する
	/// @param s 文字列
	/// @return 行動パターン
	static BehaviorKind ToBehaviorKind(const s3d::String& s);

	/// @brief 文字列をコライダー図形に変換する
	/// @param s 文字列
	/// @return コライダー図形
	static ColliderKind ToColliderKind(const s3d::String& s);

	/// @brief JSONからアニメーション仕様を読み込む
	/// @param anim JSONデータ
	/// @return アニメーション仕様。無効な場合はnullopt
	static s3d::Optional<AnimationSpec> ReadAnimationSpec(const s3d::JSON& anim);

	/// @brief 敵データのJSONルート
	s3d::JSON enemy_data_;

	/// @brief 空のJSON定数
	static const s3d::JSON kEmptyJSON;

	/// @brief 敵データのファイルパス
	static constexpr s3d::FilePathView kEnemyDataPath = U"asset/EnemyData.json";
};
