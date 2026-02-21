#pragma once

struct Asset {
    double spot;
    double volatility;
    double risk_free_rate;
};

struct PrecomputedAsset {
    double logSpot;
    double drift;
    double diffusion;
};