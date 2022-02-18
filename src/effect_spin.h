#pragma once

#include "pixeltypes.hpp"
#include "normalizedwindow.hpp"

//
// A SpinWrapper does a rotation around the center
// of the frame.
// To generalize this, a transform should be a part 
// of the SamplerWrapper
// translation, scale, rotate should be core
struct SpinWrapper : public vdj::SamplerWrapper
{
    double fCurrentAngle;
    double fPivotU=0.5;
    double fPivotV=0.5;

public:
    SpinWrapper(vdj::SourceSampler wrapped, double rads)
        : SamplerWrapper(wrapped, vdj::RectD(0,0,1,1))
        ,fCurrentAngle(rads)
    {}

    void setAngle(double rads) 
    {
        fCurrentAngle = rads;
    }

    vdj::PixelRGBA getValue(double u, double v) override
    {
        auto s = sin(fCurrentAngle);
        auto c = cos(fCurrentAngle);

        u -= fPivotU;
        v -= fPivotV;

        double nx = ((u * c) - (v * s));
        double ny = ((u * s) + (v * c));

        auto u1 = maths::Clamp(nx + fPivotU, 0,1.0);
        auto v1 = maths::Clamp(ny + fPivotV, 0, 1.0);

        // If we end up out of frame
        // return transparent
        if ((u1<0) || (u1>1.0))
            return vdj::PixelRGBA(0x0);

        if ((v1<0) || (v1>1.0))
            return vdj::PixelRGBA(0x0);

        // get value from our wrapped sampler
        vdj::PixelRGBA co = fBackground->getValue(u1, v1);

        return co;
    }

    static std::shared_ptr< SpinWrapper> create(vdj::SourceSampler wrapped, double rads = 0.0)
    {
        return std::make_shared<SpinWrapper>(wrapped, rads);
    }
};

struct SpinAnimation : public vdj::IAnimateField
{
    double beginAngle;
    double endAngle;
    std::shared_ptr< SpinWrapper> fSampler;

    SpinAnimation(std::shared_ptr< SpinWrapper> sampler, const double& beginPos, const double& endPos)
        :IAnimateField()
        ,beginAngle(beginPos)
        ,endAngle(endPos)
        , fSampler(sampler)
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


INLINE std::shared_ptr<vdj::AnimationWindow> createSpinner(double duration,
    vdj::SourceSampler s1,
    vdj::SourceSampler s2,
    const double& beginPos=0, const double& endPos=maths::Radians(360.0))
{
    // Form the backing window
    auto res = std::make_shared<vdj::AnimationWindow>(duration);
    auto backing = std::make_shared<vdj::SamplerWrapper>(s1, vdj::RectD(0, 0, 1, 1));
    res->addChild(backing);

    // Create the animator
    auto panel = SpinWrapper::create(s2);
    res->addChild(panel);

    // Create the animator
    auto animator = SpinAnimation::create(panel, beginPos, endPos);
    res->addMotion(animator);

    return res;
}

