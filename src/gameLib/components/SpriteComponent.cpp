#include "SpriteComponent.h"

#include <string>

#include "GameObject.h"
#include "GraphicsManager.h"
#include "Rect.h"
#include "Sprite.h"
#include "SpriteLoader.h"
#include "Vector2D.h"
SpriteComponent::SpriteComponent(GameObject& owner,
                                 const SpriteSheetInfo& spriteSheet,
                                 const Vector2DInt& indexInSpritesheet)
    : Component(owner),
      mSpriteSheetInfo(spriteSheet),
      mSpriteSheetIndex(indexInSpritesheet) {}

SpriteComponent::SpriteComponent(GameObject& owner, const SerializedObj& serObj)
    : Component(owner, serObj.at(SerializeString_Parent)),
      mScaleToTileGrid(serObj.at("scaleToGrid")),
      mSpriteSheetInfo(serObj.at("spritesheetInfo")),
      mSpriteSheetIndex(serObj.at("spriteSheetIndex")) {}

SerializedObj SpriteComponent::serialize() const {
  SerializedObj out = createSerializedObj<SpriteComponent>();
  out["scaleToGrid"] = mScaleToTileGrid;
  out["spritesheetInfo"] = mSpriteSheetInfo;
  out["spriteSheetIndex"] = mSpriteSheetIndex;
  out[SerializeString_Parent] = Component::serialize();
  return out;
}

void SpriteComponent::setup() {
  // mSprite =
  //     SpriteLoader::loadSpriteByIndex(mSpriteSheetInfo, mSpriteSheetIndex);
}

void SpriteComponent::teardown() {}

void SpriteComponent::render() {
  if (mSprite) {
    Vector3DInt pos{owner().getPosition()};
    GraphicsManager::renderTexture(*mSprite, pos, owner().getScale(),
                                   owner().getRotation(), mCentered, mFlip);
  }
}