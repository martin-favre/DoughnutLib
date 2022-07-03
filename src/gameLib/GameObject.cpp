#include "GameObject.h"

#include "Engine.h"

GameObject::GameObject() : mIdentifier(Uuid::generateNew()) {}

GameObject::~GameObject() {
  LOG("Deleting GameObject " << mName << " " << this);
}

bool GameObject::enabled() { return mEnabled; }
void GameObject::setEnabled(bool enabled) { mEnabled = enabled; }

void GameObject::setup() {
  for (auto& component : mComponents) {
    component->setup();
  }
}

void GameObject::teardown() {
  for (auto& component : mComponents) {
    component->teardown();
  }
}

void GameObject::updateComponents() {
  if (!mEnabled) return;
  for (auto& component : mComponents) {
    component->update();
  }
}

void GameObject::render() {
  if (!mEnabled) return;
  for (auto& component : mComponents) {
    component->render();
  }
}

void GameObject::setPosition(const Vector3DInt& pos) {
  mPosition = pos;
}

int GameObject::getRenderDepth() const { return mRenderDepth; }
void GameObject::setRenderDepth(int depth) {
  mRenderDepth = depth;
}

Vector3DInt GameObject::getPosition() const { return mPosition; }

Vector2D GameObject::getScale() const { return mScale; }
void GameObject::setScale(const Vector2D& newScale) {
  mScale = newScale;
}
double GameObject::getRotation() const { return mRotation; }

const Uuid& GameObject::getIdentifier() const { return mIdentifier; }

void GameObject::destroy() { Engine::removeGameObject(mIdentifier); }

void GameObject::setName(const std::string& name) { mName = name; }

const std::string& GameObject::name() const { return mName; }
