#pragma once

#include "pixeltypes.h"

//
// A SpinWrapper does a rotation around the center
// of the frame.
// To generalize this, a transform should be a part 
// of the SamplerWrapper
// translation, scale, rotate should be core
struct SpinWrapper : public SamplerWrapper
{
    double fCurrentAngle;

public:
    SpinWrapper(SourceSampler wrapped, double rads)
        : SamplerWrapper(wrapped, RectD(0,0,1,1))
        ,fCurrentAngle(rads)
    {}

    void setAngle(double rads) 
    {
        fCurrentAngle = rads;
    }

    PixelRGBA getValue(double u, double v) override
    {
        auto s = sin(fCurrentAngle);
        auto c = cos(fCurrentAngle);

        u -= 0.5;
        v -= 0.5;

        double nx = ((u * c) - (v * s));
        double ny = ((u * s) + (v * c));

        auto u1 = maths::Clamp(nx + 0.5, 0,1.0);
        auto v1 = maths::Clamp(ny + 0.5, 0, 1.0);

        // get value from our wrapped sampler
        PixelRGBA co = fBackground->getValue(u1, v1);

        return co;
    }

    static std::shared_ptr< SpinWrapper> create(SourceSampler wrapped, double rads = 0.0)
    {
        return std::make_shared<SpinWrapper>(wrapped, rads);
    }
};

struct SpinAnimation : public IAnimateField
{
    double beginAngle;
    double endAngle;
    std::shared_ptr< SpinWrapper> fSampler;

    SpinAnimation(std::shared_ptr< SpinWrapper> sampler, const double& beginPos, const double& endPos)
        :IAnimateField()
        , fSampler(sampler)
        ,beginAngle(beginPos)
        ,endAngle(endPos)
    {}

    void onUpdate(double u) override
    {
        auto rads = maths::Lerp(u, beginAngle, endAngle);
        fSampler->setAngle(rads);
    }

    static std::shared_ptr< SpinAnimation> create(std::shared_ptr< SpinWrapper> sampler, const double& beginPos, const double& endPos)
    {
        return std::make_shared< SpinAnimation>(sampler, beginPos, endPos);
    }
};


INLINE std::shared_ptr<AnimationWindow> createSpinner(double duration,
    SourceSampler s1,
    SourceSampler s2,
    const double& beginPos=0, const double& endPos=maths::Radians(360.0))
{
    // Form the backing window
    auto res = std::make_shared<AnimationWindow>(duration);
    auto backing = std::make_shared<SamplerWrapper>(s1, RectD(0, 0, 1, 1));
    res->addChild(backing);

    // Create the animator
    auto panel = SpinWrapper::create(s2);
    res->addChild(panel);

    // Create the animator
    auto animator = SpinAnimation::create(panel, beginPos, endPos);
    res->addMotion(animator);

    return res;
}

