#include "ofApp.h"

std::vector<ofVec3f> srcPoints_;

//--------------------------------------------------------------
void ofApp::setup()
{
    ofBackground(0);
    
    mVidGrabber.setup(1280, 720);
    
    mDivH = 1;
    mDivV = 1;
    initMesh(mDivH, mDivV);
    
    mSrcRect.set(0, 0, mVidGrabber.getWidth(), mVidGrabber.getHeight());
    
    mSrcPoints.emplace_back(mSrcRect.getTopLeft());
    mSrcPoints.emplace_back(mSrcRect.getTopRight());
    mSrcPoints.emplace_back(mSrcRect.getBottomRight());
    mSrcPoints.emplace_back(mSrcRect.getBottomLeft());
    
    mDstPoints.emplace_back(ofVec3f((ofGetWidth() - mSrcRect.getWidth()) * 0.5, (ofGetHeight() - mSrcRect.getHeight()) * 0.5));
    mDstPoints.emplace_back(mDstPoints.at(0) + mSrcRect.getTopRight());
    mDstPoints.emplace_back(mDstPoints.at(0) + mSrcRect.getBottomRight());
    mDstPoints.emplace_back(mDstPoints.at(0) + mSrcRect.getBottomLeft());
    
    ofxHomographyHelper::findHomography(mSrcPoints, mDstPoints, mHomography);
    
    mTargetIndex = 0;
    mIndexOfTweakingTargetDstPoint = -1;
    
    mIndex = 0;
    bSetAnchorPoint = false;
    bTargetSelected = false;
    bTweaking = false;
}

//--------------------------------------------------------------
void ofApp::update()
{
    mVidGrabber.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofPushMatrix();
    {
        glMultMatrixf(mHomography.getPtr());

        mVidGrabber.getTexture().bind();
        mMesh.draw();
        mVidGrabber.getTexture().unbind();
        
        mMesh.drawWireframe();
    }
    ofPopMatrix();
    
    // dest points
    ofPushStyle();
    {
        ofSetColor(ofColor::cyan);
        ofNoFill();
        
        for (auto i = 0; i < mDstPoints.size(); ++i)
        {
            ofSetLineWidth(mIndexOfTweakingTargetDstPoint == i ? 3 : 1);
            ofDrawCircle(mDstPoints.at(i), 25);
        }
    }
    ofPopStyle();
    
    ofVec3f mouse_(ofGetMouseX(), ofGetMouseY(), 0);
    
    for (auto i = 0; i < mMesh.getNumVertices(); ++i)
    {
        ofVec3f v_ = mMesh.getVertices().at(i) * mHomography;
        
        ofPushStyle();
        {
            ofFill();
            
            if (v_.distance(mouse_) < 10)
            {
                ofSetColor(ofColor::red);
            }
            else
            {
                ofSetColor(ofColor::white);
            }
            
            ofDrawCircle(v_, 10);
        }
        ofPopStyle();
    }
    
    ofBeginShape();
    {
        ofSetColor(ofColor::cyan, 80);
        ofFill();
        
        for (auto& p_ : mDstPoints)
        {
            ofVertex(p_);
        }
    }
    ofEndShape();
    
    ofPushMatrix();
    {
        ofTranslate(200, 200);
        ofScale(0.25, 0.25);
        
        mVidGrabber.getTexture().bind();
        mMesh.draw();
        mVidGrabber.getTexture().unbind();
        
        mMesh.drawWireframe();
        
        ofBeginShape();
        {
            ofSetColor(ofColor::cyan, 80);
            ofFill();
            
            for (auto& p_ : mSrcPoints)
            {
                ofVertex(p_);
            }
        }
        ofEndShape();
    }
    ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::initMesh(int divH, int divV)
{
    auto horizontalResolution = mVidGrabber.getWidth();
    auto verticalResolution = mVidGrabber.getHeight();
    auto cellWidth = horizontalResolution / divH;
    auto cellHeight = verticalResolution / divV;
    auto numHorizontalCells = horizontalResolution / cellWidth + 1;
    auto numVerticalCells = verticalResolution / cellHeight + 1;
    
    mMesh.clear();
    mMesh.setMode(OF_PRIMITIVE_TRIANGLES);
    mMesh.setUsage(GL_DYNAMIC_DRAW);
    
    for (auto y_ = 0; y_ < numVerticalCells; ++y_)
    {
        for (auto x_ = 0; x_ < numHorizontalCells; ++x_)
        {
            mMesh.addVertex(ofVec3f(x_ * cellWidth, y_ * cellHeight, 0));
            mMesh.addTexCoord(ofVec3f(x_ * cellWidth, y_ * cellHeight));
            mMesh.addColor(ofColor(255, 255));
        }
    }
    
    for (auto y_ = 0; y_ < numVerticalCells - 1; ++y_)
    {
        for (auto x_ = 0; x_ < numHorizontalCells - 1; ++x_)
        {
            int i1_ = x_ + numHorizontalCells * y_;
            int i2_ = x_ + 1 + numHorizontalCells * y_;
            int i3_ = x_ + numHorizontalCells * (y_ + 1);
            int i4_ = x_ + 1 + numHorizontalCells * (y_ + 1);
            mMesh.addTriangle( i1_, i2_, i3_ );
            mMesh.addTriangle( i2_, i4_, i3_ );
        }
    }
    
    mCornerPointIndices.clear();
    mCornerPointIndices.emplace_back(0);
    mCornerPointIndices.emplace_back(divH);
    mCornerPointIndices.emplace_back(divH * divV + (divH + divV));
    mCornerPointIndices.emplace_back(divV * (divH + 1));
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
    if (bSetAnchorPoint)
    {
        return;
    }
    
    auto mouse_ = ofVec3f(x, y);
    
    if (-1 < mIndexOfTweakingTargetDstPoint || -1 < mIndexOfTweakingTargetMeshVertex)
    {
        bTweaking = true;
    }
    else
    {
        for (auto i = 0; i < mMesh.getNumVertices(); ++i)
        {
            auto v_ = mMesh.getVertices().at(i) * mHomography;
            
            if (10 > v_.distance(mouse_))
            {
                mIndexOfTweakingTargetMeshVertex = i;
                bTweaking = true;
                break;
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
    if (bSetAnchorPoint)
    {
        mDstPoints.at(mIndex).set(x, y);
        
        if (3 == mIndex)
        {
            ofxHomographyHelper::findHomography(mSrcPoints, mDstPoints, mHomography);
        }
        
        ++mIndex %= 4;
    }
    
    bTweaking = false;
    mIndexOfTweakingTargetDstPoint = -1;
    mIndexOfTweakingTargetMeshVertex = -1;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
    if (!bTweaking)
    {
        return;
    }
    
    auto mouse_ = ofVec3f(x, y);
    
    if (-1 < mIndexOfTweakingTargetDstPoint)
    {
        mDstPoints.at(mIndexOfTweakingTargetDstPoint).set(x, y);
        ofxHomographyHelper::findHomography(mSrcPoints, mDstPoints, mHomography);
    }
    else
    {
        if (-1 < mIndexOfTweakingTargetMeshVertex)
        {
            ofMatrix4x4 homography_ = ofxHomographyHelper::findHomography(mDstPoints, mSrcPoints);
            auto v_ = homography_ * mouse_;
            mMesh.setVertex(mIndexOfTweakingTargetMeshVertex, v_);
            
            auto minX = mMesh.getVertex(0).x;
            auto minY = mMesh.getVertex(0).y;
            auto maxX = mMesh.getVertex(mMesh.getNumVertices() - 1).x;
            auto maxY = mMesh.getVertex(mMesh.getNumVertices() - 1).y;
            
            for (auto i = 0; i < mMesh.getNumVertices(); ++i)
            {
                auto src_ = mMesh.getVertex(i);
                minY = std::min(minY, src_.y);
                maxY = std::max(maxY, src_.y);
                minX = std::min(minX, src_.x);
                maxX = std::max(maxX, src_.x);
            }
            
            mSrcPoints.at(0).set(minX, minY);
            mSrcPoints.at(1).set(maxX, minY);
            mSrcPoints.at(2).set(maxX, maxY);
            mSrcPoints.at(3).set(minX, maxY);
            
            mDstPoints.at(0).set(mSrcPoints.at(0) * mHomography);
            mDstPoints.at(1).set(mSrcPoints.at(1) * mHomography);
            mDstPoints.at(2).set(mSrcPoints.at(2) * mHomography);
            mDstPoints.at(3).set(mSrcPoints.at(3) * mHomography);
            
            ofxHomographyHelper::findHomography(mSrcPoints, mDstPoints, mHomography);
        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{
    if (bTweaking)
    {
        return;
    }
    
    for (auto i = 0; i < mDstPoints.size(); ++i)
    {
        auto dist_ = mDstPoints.at(i).distance(ofVec3f(x, y));
                                               
        if (10 > dist_)
        {
            mIndexOfTweakingTargetMeshVertex = mCornerPointIndices.at(i);
            mIndexOfTweakingTargetDstPoint = -1;
            break;
        }
        else if (25 > mDstPoints.at(i).distance(ofVec3f(x, y)))
        {
            mIndexOfTweakingTargetDstPoint = i;
            mIndexOfTweakingTargetMeshVertex = -1;
            break;
        }
        else
        {
            mIndexOfTweakingTargetDstPoint = -1;
            mIndexOfTweakingTargetMeshVertex = -1;
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    if ('f' == key)
    {
        ofToggleFullscreen();
    }
    
    if ('s' == key)
    {
        bSetAnchorPoint = true;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
    if ('s' == key)
    {
        bSetAnchorPoint = false;
    }
    
    if (OF_KEY_UP == key)
    {
        ++mDivV;
        initMesh(mDivH, mDivV);
    }
    else if (OF_KEY_DOWN == key)
    {
        mDivV = std::max(1, --mDivV);
        initMesh(mDivH, mDivV);
    }
    
    if (OF_KEY_RIGHT == key)
    {
        ++mDivH;
        initMesh(mDivH, mDivV);
    }
    else if (OF_KEY_LEFT == key)
    {
        mDivH = std::max(1, --mDivH);
        initMesh(mDivH, mDivV);
    }
    
    if ('a' == key) // add
    {
        
    }
}