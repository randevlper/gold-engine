#include "playerController.h"

#include <SDL_keycode.h>

#include "Engine/Core/Context.h"
#include "Engine/Core/World.h"
#include "Engine/Core/WorldManager.h"
#include "Engine/Nodes/SpriteRenderer.h"
#include "Engine/Utilities/Debug.h"
#include "Engine/Core/Input.h"

#include "Engine/UI/Text.h"

#include "net.h"
#include "hook.h"
#include "fish.h"

PlayerController::PlayerController()
{
	_type = "PlayerController";
	_net = nullptr;
	_hook = nullptr;
	_spriteRenderer = nullptr;

	_horizontalSpeed = 400;
	_hookVerticalSpeed = 500;
	_hookAutoReal = 100;

	_minXPos = -940;
	_maxXPos = 940;

	_hookMaxHeight = 400;
	_hookMaxDepth = -400;

	_score = 0;
	_scoreText = nullptr;

	_state = PlayerState::FISHING;
	_fishCatching = 0;
}

void PlayerController::init() {
	World* world = me::WorldManager::getWorld();
	_net = world->get<Net>("PlayerNet");
	_hook = world->get<Hook>("PlayerHook");

	_scoreText = world->get<me::ui::Text>("ScoreText");
	_scoreText->setText("Score: 0");

	_buttonPressBackground = world->get<SpriteRenderer>("ButtonPromtSprite");
	_buttonTextPromt = world->get<me::ui::Text>("ButtonPromtText");
	//ButtonPromtSprite
	//ButtonPromtText
	_buttonPressBackground->setIsEnabled(false);
	_buttonTextPromt->setIsEnabled(false);
	_buttonTextPromt->setText("Z");

	if (_net) {
		_net->CaughtFish = std::bind(&PlayerController::OnNetCatch, this, std::placeholders::_1);
	}
}

void PlayerController::tick()
{
	switch (_state)
	{
	case PlayerState::FISHING:
		fishing();
		break;
	case PlayerState::CATCHING:
		catching();
		break;
	default:
		break;
	}
}

void PlayerController::OnNetCatch(std::vector<Fish*> fishes)
{
	_fishToCatch = fishes;
	_state = PlayerState::CATCHING;

	_hook->setVelocity({ 0,0 });
	_net->setVelocity({ 0,0 });

	_buttonPressBackground->setIsEnabled(true);
	_buttonTextPromt->setIsEnabled(true);
	_buttonTextPromt->setText(_fishToCatch[_fishCatching]->getFishData().scancodeName);
}

void PlayerController::fishing()
{
	float delta = Context::getDeltaTime();

	glm::vec2 dir = { 0,0 };
	if (Input::getKey(SDL_SCANCODE_A)) {
		dir.x += -1;
	}
	if (Input::getKey(SDL_SCANCODE_D)) {
		dir.x += 1;
	}
	if (Input::getKey(SDL_SCANCODE_W)) {
		dir.y += 1;
	}
	if (Input::getKey(SDL_SCANCODE_S)) {
		dir.y += -1;
	}

	float horizontalMovement = dir.x * _horizontalSpeed;

	if (_transform.getLocalPosition().x > _maxXPos && horizontalMovement > 0) {
		horizontalMovement = 0;
	}
	if (_transform.getLocalPosition().x < _minXPos && horizontalMovement < 0) {
		horizontalMovement = 0;
	}

	_transform.translate({ horizontalMovement * delta,0 });
	//TODO Properly implement syncronization of Transform2D and Body2D
	_net->setVelocity({ horizontalMovement, 0 });

	float hookMovement = _hookAutoReal;  
	if (dir.y != 0) {
		hookMovement = ( hookMovement + _hookVerticalSpeed) * dir.y;
	}

	//Out of bounds and trying to go up
	if (_hook->getPosition().y > _hookMaxHeight && hookMovement > 0) {
		hookMovement = 0;
	}
	//Out of bounds and trying to go down
	if (_hook->getPosition().y < _hookMaxDepth && hookMovement < 0) {
		hookMovement = 0;
	}

	_hook->setVelocity({ dir.x * _horizontalSpeed, hookMovement });
	//Move hook up and down, move everything side to side
	//setVelocity(dir);
}

void PlayerController::catching()
{
	//If catch any fish, stop movement and pop up the input promts
	//Stop movement
	//Show button promt
	//Green on correct, red on wrong
		static bool hasBeenPressed = false;
		if (Input::getAnyKeyPressed(79, 82) && !hasBeenPressed) {
			if (Input::getKeyDown(_fishToCatch[_fishCatching]->getFishData().scancode)) {
				_score += _fishToCatch[_fishCatching]->getScore();
				_scoreText->setText("Score: " + std::to_string(_score));
				_buttonPressBackground->setColor({ 0,1,0,1 });
				//Debug::Log("Correct!");
			}
			else {
				_buttonPressBackground->setColor({ 1,0,0,1 });
			}
			World* world = me::WorldManager::getWorld();
			world->destroy(_fishToCatch[_fishCatching]);
			_fishCatching++;
			hasBeenPressed = true;

			if (_fishCatching >= _fishToCatch.size()) {
				_fishCatching = 0;
				_fishToCatch = std::vector<Fish*>();
				_state = PlayerState::FISHING;

				_buttonPressBackground->setIsEnabled(false);
				_buttonTextPromt->setIsEnabled(false);
				return;
			}

			_buttonTextPromt->setText(_fishToCatch[_fishCatching]->getFishData().scancodeName);
		}
		else if (!Input::getAnyKeyPressed(79, 82)) {
			hasBeenPressed = false;
		}
}