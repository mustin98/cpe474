#pragma once
#ifndef _LINK_H_
#define _LINK_H_

#include <vector>

class Link {
public:
   Shape();
   virtual ~Shape();

   Link &parent;
   std::vector<Link> children;
   Eigen::Matrix4f Ep;
   Eigen::Matrix4f Em;
   float angle;
};

#endif
