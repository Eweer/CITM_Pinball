#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include "App.h"
#include "Textures.h"
#include "Defs.h"

#include <vector>

struct SDL_Texture;

enum class AnimIteration
{
	ONCE = 0,
	LOOP_FROM_START,
	FORWARD_BACKWARD,
	LOOP_FORWARD_BACKWARD,
	NEVER,
	UNKNOWN
};

class Animation
{
public:

	Animation() = default;

	~Animation() = default;

	SDL_Texture *GetCurrentFrame()
	{
		//if it's not active, we just return frame
		if(!bActive)
		{
			if(staticImage) return staticImage;
			return frames[(int)currentFrame];
		}

		//if it's active and finished, it's no longer finished
		if(bFinished) bFinished = !bFinished;

		//if it's active we increase the frame
		currentFrame += speed;

		//if no more animations in std::vector<SDL_Texture*> frames
		if((uint)currentFrame >= frames.size() + 1 || (int)currentFrame < 0)
		{
			//we do things
			switch(animStyle)
			{
				case AnimIteration::FORWARD_BACKWARD:
					if(speed > 0) currentFrame = (float)frames.size() - 1;
					else Stop();

					speed *= -1;
					break;

				case AnimIteration::ONCE:
					Stop();
					break;

				case AnimIteration::LOOP_FROM_START:
					currentFrame = 0.0f;
					break;

				case AnimIteration::LOOP_FORWARD_BACKWARD:
					currentFrame = (speed < 0) ? 0 : (float)frames.size() - 1;
					speed *= -1;
					break;

				default:
					Stop();
			}
			bFinished = true;
		}
		if((int)currentFrame >= frames.size()) 
			return frames[(int)frames.size() - 1];
		else 
			return frames[(int)currentFrame];
	}

	Animation *AddStaticImage(const char *pathToPNG)
	{
		staticImage = app->tex->Load(pathToPNG);
		return this;
	}

	Animation* AddSingleFrame(const char* pathToPNG)
	{
		frames.push_back(app->tex->Load(pathToPNG));
		return this;
	}

	Animation* AddSingleFrame(SDL_Texture* texture)
	{
		frames.push_back(texture);
		return this;
	}

	bool CleanUp()
	{
		for(auto &elem : frames) app->tex->UnLoad(elem);
		if(staticImage) app->tex->UnLoad(staticImage);
		return true;
	}

	bool GetAnimFinished() const
	{
		return bFinished;
	}

	void SetSpeed(float const &animSpeed)
	{
		speed = animSpeed;
	}

	void SetAnimStyle(int i)
	{
		animStyle = static_cast<AnimIteration>(i);
	}

	void SetAnimStyle(AnimIteration i)
	{
		animStyle = i;
	}

	AnimIteration GetAnimStyle() const
	{
		return animStyle;
	}
	
	void Start()
	{
		bActive = true;
	}

	void Pause()
	{
		bActive = false;
	}

	void Reset()
	{
		bFinished = false;
		currentFrame = 0;
	}

	void Stop()
	{
		Pause();
		Reset();
	}

private:
	float speed = 0;
	float currentFrame = 0;
	AnimIteration animStyle = AnimIteration::NEVER;
	bool bActive = false;
	bool bFinished = false;
	std::vector<SDL_Texture*> frames;
	SDL_Texture *staticImage = nullptr;
}; 
#endif	// __ANIMATION_H__