#pragma once

#include "ofMain.h"
#include "ofxHomographyHelper.h"

class ofApp : public ofBaseApp
{
public:
    void setup();
    void update();
    void draw();
    
    void initMesh(int divH = 4, int divV = 4);
    
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseDragged(int x, int y, int button);
    void mouseMoved(int x, int y);
    void keyPressed(int key);
    void keyReleased(int key);
    
    ofVideoGrabber mVidGrabber;
    ofVboMesh mMesh;
    ofIndexType mTargetIndex;
    int mIndexOfTweakingTargetDstPoint;
    int mIndexOfTweakingTargetMeshVertex;
    int mDivH, mDivV;
    
    ofRectangle mSrcRect;
    std::vector<ofVec3f> mSrcPoints, mDstPoints;
    std::vector<ofIndexType> mCornerPointIndices;
    ofMatrix4x4 mHomography;
    int mIndex;
    
    bool bSetAnchorPoint;
    bool bTargetSelected;
    bool bTweaking;
};
