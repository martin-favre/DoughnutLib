#include "TextComponent.h"

#include "Font.h"
#include "GameObject.h"
#include "GraphicsManager.h"
#include "SpriteLoader.h"
#include "Vector2DInt.h"

TextComponent::TextComponent(GameObject &owner, const std::string &pathToFont,
                             int size)
    : Component(owner), mFontSource(pathToFont),
      mSize(size), mFont(SpriteLoader::loadFont(pathToFont, size)) {}

TextComponent::TextComponent(GameObject &owner, const SerializedObj &serObj)
    : Component(owner, serObj["parent"]), mFontSource(serObj["fontSource"]),
      mSize(serObj["size"]), mFont(SpriteLoader::loadFont(mFontSource, mSize)) {}

SerializedObj TextComponent::serialize() const {
  SerializedObj out = createSerializedObj<TextComponent>();
  out["fontSource"] = mFontSource;
  out["size"] = mSize;
  out["parent"] = Component::serialize();
  return out;
}

void TextComponent::setFontSize(int size) {
  mSize = size;
  mFont = SpriteLoader::loadFont(mFontSource, size);
}

std::string TextComponent::getText() { return mText; }

void TextComponent::setText(const std::string &text) {
  if (text != mText) {
    size_t lastLinebreak = 0;
    size_t indx = 0;
    mRequestedSprites.clear();
    while (indx != text.npos) {
      indx = text.find("\n", lastLinebreak);
      std::string subString = text.substr(lastLinebreak, indx - lastLinebreak);
      if (subString != "") {
        mRequestedSprites.emplace_back(
            SpriteLoader::getSpriteFromTextFast(subString, *mFont, mColor));
      }
      lastLinebreak = indx + 1;
    }
    mText = text;
  }
}

void TextComponent::setColor(const SDL_Color &color) {
  mColor = color;
}

void TextComponent::teardown() {}

void TextComponent::render() {
  ASSERT(mFont.get(), "Received null Font ptr");
  ASSERT(mFont->getSdlFont(), "Received null TTF_Font ptr");
  if (mRequestedSprites.size()) {
    mSprites.clear();
    for (auto &sprite : mRequestedSprites) {
      mSprites.emplace_back(std::move(sprite));
    }
    mRequestedSprites.clear();
  }

  int fontHeight = TTF_FontHeight(mFont->getSdlFont());
  int heightOffset = 0;
  for (const auto &sprite : mSprites) {
    if (sprite->getSdlTexture() != nullptr) {
      const GameObject &gObj = owner();
      Vector2DInt pos{gObj.getPosition()};
      pos.y += heightOffset;
      GraphicsManager::renderTexture(*sprite, pos, gObj.getScale(),
                                     gObj.getRotation(), mCentered, mFlip);
      heightOffset += fontHeight;
    }
  }
}

std::string TextComponent::getTypeString() { return "TextComponent"; }