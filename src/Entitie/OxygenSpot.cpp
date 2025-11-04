#include "Component/Animation.h"
#include "OxygenSpot.h"

#include <Siv3D.hpp>
#include <variant>

OxygenSpot::OxygenSpot(const Vec2& center_pos, const Vec2& size)
	: pos_(center_pos)
	, size_(size)
	, collider_(Collider{ RectF{ Arg::center(center_pos), size }, ColliderTag::kOxygen })
{
	SetupAnimation();
	anim_controller_.Play(U"idle");
}

void OxygenSpot::SetupAnimation()
{
	Animation anim;
	anim.texture_asset_names = {
		U"hot-spring-and-bubble2",
		U"hot-spring-and-bubble3",
		U"hot-spring-and-bubble4",
		U"hot-spring-and-bubble5",
		U"hot-spring-and-bubble6",
		U"hot-spring-and-bubble7",
	};
	anim.frame_duration_sec = 0.2;
	anim.is_looping = true;
	anim_controller_.AddAnimation(U"idle", anim);
}

void OxygenSpot::Update()
{
	anim_controller_.Update();

	// コライダーの中心をスポット位置に追従させる
	UpdateColliderCenter();
}

void OxygenSpot::UpdateColliderCenter()
{
	// variant に格納される形状の種類ごとに中心設定を行う
	std::visit([&](auto& shape)
			   {
				   using T = std::decay_t<decltype(shape)>;
				   if constexpr(std::is_same_v<T, s3d::Circle> || std::is_same_v<T, s3d::RectF>)
				   {
					   shape.setCenter(pos_);
				   }
				   // 他の形状タイプには対応していないが，必要ならここに追加
			   }, collider_.shape);
}

void OxygenSpot::Draw(const Vec2& camera_offset) const
{
	if(auto texture_asset = anim_controller_.GetCurrentTextureAsset())
	{
		const Vec2 draw_pos = pos_ - camera_offset;
		const Vec2 final_draw_pos = s3d::Floor(draw_pos);
		texture_asset->drawAt(final_draw_pos);
	}
}

Vec2 OxygenSpot::GetPos() const { return pos_; }
