#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <optional>

namespace moderngekko::controls {
using PcMatrix = std::array<double,16>;
struct PcMenuRect {
  double left, top, right, bottom;
  bool Contains(double x,double y) const {
    return std::isfinite(x) && std::isfinite(y) &&
           x>=left && x<right && y>=top && y<bottom;
  }
};
// A native volume slider has eleven detents (0..10). Dragging beyond its
// horizontal extent clamps; malformed geometry never requests a change.
inline std::optional<int> PcVolumeTarget(double x,const PcMenuRect& bar) {
  if(!std::isfinite(x) || !std::isfinite(bar.left) || !std::isfinite(bar.right) ||
     bar.right<=bar.left)return {};
  return int(std::lround(std::clamp((x-bar.left)/(bar.right-bar.left),0.0,1.0)*10));
}
// Map client-normalized coordinates through the renderer's actual viewport.
// Letterbox/pillarbox bars and invalid dimensions never produce a menu target.
inline std::optional<std::array<double,2>> PcViewportPoint(
    double x,double y,const PcMenuRect& viewport) {
  const double w=viewport.right-viewport.left,h=viewport.bottom-viewport.top;
  if(!std::isfinite(w) || !std::isfinite(h) || w<=0 || h<=0 ||
     x<0 || y<0 || x>=1 || y>=1 || !viewport.Contains(x,y))return {};
  return std::array<double,2>{(x-viewport.left)/w,(y-viewport.top)/h};
}
// Alchemy matrices use row vectors. Bounds are world-space X, top Z, Y,
// width X, height Z, depth Y, as maintained by CMenuItem's native layout.
inline std::optional<PcMenuRect> PcProjectMenuBounds(
    const std::array<double,6>& bounds,const PcMatrix& camera_world,
    const PcMatrix& projection) {
  for(double v:bounds)if(!std::isfinite(v))return {};
  for(double v:camera_world)if(!std::isfinite(v))return {};
  for(double v:projection)if(!std::isfinite(v))return {};
  if(bounds[3]<=0 || bounds[4]<=0 || bounds[5]<0)return {};
  // Invert the actual camera matrix; do not assume a fixed menu scale or pose.
  double rows[4][8]{};
  for(unsigned r=0;r<4;++r) {
    for(unsigned c=0;c<4;++c)rows[r][c]=camera_world[r*4+c];
    rows[r][r+4]=1;
  }
  for(unsigned c=0;c<4;++c) {
    unsigned pivot=c;
    for(unsigned r=c+1;r<4;++r)if(std::abs(rows[r][c])>std::abs(rows[pivot][c]))pivot=r;
    if(std::abs(rows[pivot][c])<1e-12)return {};
    if(pivot!=c)for(unsigned k=0;k<8;++k)std::swap(rows[pivot][k],rows[c][k]);
    const double divisor=rows[c][c];
    for(double& v:rows[c])v/=divisor;
    for(unsigned r=0;r<4;++r)if(r!=c) {
      const double factor=rows[r][c];
      for(unsigned k=0;k<8;++k)rows[r][k]-=factor*rows[c][k];
    }
  }
  PcMatrix vp{};
  for(unsigned r=0;r<4;++r)for(unsigned c=0;c<4;++c)
    for(unsigned k=0;k<4;++k)vp[r*4+c]+=rows[r][k+4]*projection[k*4+c];
  PcMenuRect rect{1,1,0,0};
  for(unsigned corner=0;corner<8;++corner) {
    const std::array<double,4> point{bounds[0]+((corner&1)?bounds[3]:0),
      bounds[2]+((corner&2)?bounds[5]:0),bounds[1]-((corner&4)?bounds[4]:0),1};
    std::array<double,4> clip{};
    for(unsigned c=0;c<4;++c)for(unsigned k=0;k<4;++k)clip[c]+=point[k]*vp[k*4+c];
    if(!std::isfinite(clip[3]) || clip[3]<=1e-9)return {};
    const double x=(clip[0]/clip[3]+1)*.5, y=(1-clip[1]/clip[3])*.5;
    if(!std::isfinite(x) || !std::isfinite(y))return {};
    rect.left=std::min(rect.left,x);rect.right=std::max(rect.right,x);
    rect.top=std::min(rect.top,y);rect.bottom=std::max(rect.bottom,y);
  }
  rect.left=std::max(0.0,rect.left);rect.top=std::max(0.0,rect.top);
  rect.right=std::min(1.0,rect.right);rect.bottom=std::min(1.0,rect.bottom);
  if(rect.left>=rect.right || rect.top>=rect.bottom)return {};
  return rect;
}
} // namespace moderngekko::controls
