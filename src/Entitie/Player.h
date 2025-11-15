#pragma once

/// @file Player.h
/// @brief プレイヤークラスの定義

#include "../Component/AnimationController.h"
#include "../Component/Collider.h"
#include "../Component/SoundController.h"
#include "../World/Stage.h"

#include <Siv3D.hpp>

/// @brief プレイヤーを管理する
class Player
{
public:
	/// @brief コンストラクタ
	Player();

	/// @brief プレイヤーを更新する
	/// @param stage ステージ情報
	void Update(const Stage& stage);

	/// @brief プレイヤーを描画する
	/// @param camera_offset カメラオフセット
	void Draw(const Vec2& camera_offset) const;

	/// @brief 位置を取得する
	/// @return プレイヤーの位置
	Vec2 GetPos() const;

	/// @brief 位置を設定する
	/// @param new_pos 設定する位置
	void SetPos(const Vec2& new_pos);

	/// @brief 現在の酸素量を取得する
	/// @return 酸素量
	double GetOxygen() const;

	/// @brief 最大酸素量を取得する
	/// @return 最大酸素量
	double GetMaxOxygen() const;

	/// @brief 酸素が空かを判定する
	/// @return 空の場合はtrue
	bool IsOxygenEmpty() const;

	/// @brief 酸素を回復する
	void RecoverOxygen();

	/// @brief 指定位置にリスポーンする
	/// @param spawn_pos リスポーン位置
	void Respawn(const Vec2& spawn_pos);

	/// @brief エンディングを開始する
	/// @param camera_center_world_x カメラ中心のワールド座標(X)
	void StartEnding(double camera_center_world_x);

	/// @brief コライダー
	Collider collider{ RectF{0, 0, 1.0, 1.0}, ColliderTag::kPlayer };

private:
	/// @brief 入力を処理する
	void HandleInput();

	/// @brief 物理演算を更新する
	/// @param stage ステージ情報
	void UpdatePhysics(const Stage& stage);

	/// @brief 重力を適用する
	void ApplyGravity();

	/// @brief 摩擦を適用する
	void ApplyFriction();

	/// @brief X軸方向の移動処理
	/// @param stage ステージ情報
	void MoveX(const Stage& stage);

	/// @brief Y軸方向の移動処理
	/// @param stage ステージ情報
	void MoveY(const Stage& stage);

	/// @brief コライダー位置を更新する
	void UpdateColliderPosition();

	/// @brief アニメーションを更新する
	void UpdateAnimation();

	/// @brief ダメージを受ける
	void TakeDamage();

	/// @brief 酸素を更新する
	void UpdateOxygen();

	/// @brief 酸素量を変更する
	/// @param amount 変更量
	void ModifyOxygen(double amount);

	/// @brief アニメーションをセットアップする
	void SetupAnimations();

	/// @brief 泳ぎ入力時の処理
	void OnSwimPressed();

	/// @brief 衝突処理を実行する
	void HandleCollisions();

	Vec2 pos_{ 0.0, 0.0 };
	Vec2 velocity_{ 0.0, 0.0 };

	bool is_moving_x_ = false;
	bool is_grounded_ = false;
	bool is_facing_right_ = false;

	bool is_invincible_ = false;
	s3d::Stopwatch invincible_timer_;
	static constexpr double kInvincibleDurationSec = 2.7;	// 無敵時間
	static constexpr int32 kBlinkIntervalMs = 300;			// 点滅の間隔
	static constexpr int32 kBlinkOnDurationMs = 150;		// 点滅中の表示時間

	bool just_took_damage_ = false;

	// 地形衝突(Physics)用のサイズ(ハーフ)
	static constexpr double kPhysicsHalfWidth = 25.0;
	static constexpr double kPhysicsHalfHeight = 62.0;

	// 敵との当たり判定(Collider)用のサイズ
	static constexpr double kColliderWidth = 60.0;
	static constexpr double kColliderHeight = 80.0;

	// 描画オフセット(スプライトの中心から左上までの距離)
	static constexpr Vec2 kDrawOffset = { 64.0, 64.0 };
	static constexpr Vec2 kEndingDrawOffset = { 96.0, 128.0 };

	// 物理パラメータ
	double horizontal_accel_{ 0.6 };
	double horizontal_speed_max_{ 1.3 };
	double friction_{ 0.90 };					// 水平方向の抵抗係数(1.0が無抵抗)
	double gravity_ = 0.06;
	double swim_power_ = -1.8;					// 水中での上昇力
	double terminal_velocity_y_ = 1.0;			// Y軸の終端速度
	double rising_gravity_multiplier_ = 0.4;	// 上昇時の重力軽減倍率(0.0で無重力)

	double oxygen_{ 100.0 };		// 酸素量(体力と同義)
	bool is_oxygen_empty_ = false;	// oxygen_ == 0でtrue

	bool is_in_ending_ = false;
	s3d::Stopwatch ending_timer_;
	static constexpr double kEndingAnimationDelaySec = 7.0;

	// エンディング中のx軸ワープ制御
	double ending_target_x_ = 0.0;			// 目標x（ワールド座標）
	bool ending_warp_enabled_ = false;		// ワープ処理中ならtrue
	double ending_warp_lerp_ = 0.01;		// Lerpファクター（1.0で即時ワープ）
	double ending_snap_threshold_ = 1.0;	// この距離以下でスナップ

	static constexpr double kMaxOxygen = 100.0;
	static constexpr double kOxygenDrainPerFrame = 0.013;	// 毎フレームの減少量
	static constexpr double kOxygenDamageAmount = 20.0;		// ダメージ時の減少量

	static constexpr double kOxygenHorizontalExtraDrain = 0.006;	// 水平移動時の追加酸素消費量
	static constexpr double kOxygenSwimCost = 0.5;					// swim時の酸素消費量
	static constexpr double kOxygenRecoveryPerFrame = 1.0;			// 回復速度(1秒あたり)

	/// @brief アニメーションコントローラー
	AnimationController anim_controller_;

	/// @brief サウンドコントローラー
	SoundController sound_controller_;
};
