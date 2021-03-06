#include "MyUI.h"
#include "cocos2d.h"
using namespace cocos2d;
void DrawShapeExt::drawSolidCircle(const CCPoint& center, float radius, float angle, unsigned int segments, bool drawLineToCenter, float scaleX, float scaleY)
{
     ccDrawCircle(center,radius,angle,segments,drawLineToCenter,scaleX,scaleY);
}

DrawShapeExt* DrawShapeExt::push(const CCPoint &point)
{
    points.push_back(point);
    return this;
}

void DrawShapeExt::drawPloy(bool tag,const ccColor4F &color)
{
    if (points.empty()) return;
    // ccColor4F color = {0.2,0.4,0.6,1};
    if (!tag)
        ccDrawPoly(&points[0], points.size(), true);
    else
        ccDrawSolidPoly(&points[0], points.size(),color);
}

DrawShapeExt* DrawShapeExt::setLineWidth(float width)
{
    glLineWidth(width);
    return this;
}

DrawShapeExt * DrawShapeExt::setShapeType(int shapeType)
{
    this->shapeType = shapeType;
    return this;
}
bool DrawShapeExt::checkIn(const CCPoint &point)
{
    if (shapeType == POLY)
    {
        std::vector<CCPoint> &polys = points;
        if (polys.size() < 3) return false;
        double testValue = 0;
        polys.push_back(polys[0]);
        for (unsigned int i = 0; i < polys.size() -1;++i)
        {
            CCPoint v0 = polys[i];
            CCPoint v1 = polys[i+1];
            double value = (point.y - v0.y) * (v1.x - v0.x) - (point.x - v0.x) * (v1.y - v0.y);
            if (testValue == 0)
            {
                testValue = value;
            }
            else
            {
                if (testValue == 0 || value == 0)
                {
                    polys.pop_back();
                    return true;
                }
                if ((testValue/fabs(testValue)) * (value/fabs(value)) < 0)
                {
                    polys.pop_back();
                    return false;
                }
            }
        }
        polys.pop_back();
        return true;
    }
    return false;
}
void DrawShapeExt::show()
{
    //ccColor4F color = {0.2,0.4,0.6,1};
    ccDrawColor4F(color.r, color.g, color.b, color.a);
    switch (shapeType) {
        case CIRCLE:
            if (points.size() != 4) return ;
            ccDrawCircle(points[0], points[1].x, points[1].y, points[2].x, points[2].y, points[3].x, points[3].y);
            break;
        case LINE:
            if (points.size() != 2) return;
            ccDrawLine(points[0], points[1]);
            break;
        case SHAPE_RECT:
            if (points.size() == 2)
                ccDrawSolidRect(points[0], points[1], color);
            break;
        case POLY:
            if (points.size())
                ccDrawSolidPoly(&points[0], points.size(),color);
        default:
            break;
    }
    ccDrawColor4F(1, 1, 1, 1);
    glLineWidth(1);
}

DrawShapeExt* DrawShapeExt::setColor(float r,float g,float b,float alpha)
{
    color.r = r / 255;
    color.g = g / 255;
    color.b = b/255;
    color.a = alpha/255;
    return this;
}

void DrawShapeExt::clear()
{
    points.clear();
}
namespace myui{
    /////////////////////////////////////////////////////////////////////////////////////////////
    // UI 的Base类 提供touch的分发 与 纪录
    // 控制控件的位置
    /////////////////////////////////////////////////////////////////////////////////////////////
    /**
	 * 设置位置
	 */
	void Base::setLocation(AlignType alignType,const CCSize &splitSize,const CCPoint &gridLocation)
	{
		if (panel)
		{
			CCPoint point = panel->getPoint(alignType,splitSize,gridLocation);
			if (this)
			{
				this->setPosition(point);
			}
		}
	}
    void Base::bindTouchHandle(TouchHandle *touchHandle)
    {
        this->touchHandle = touchHandle;
        touchHandle->retain();
    }
    bool Base::attachTouch(TouchEvent event,CCTouch *touch)
    {
        if (!this->isVisible()) return false;
        bool tag = checkIn(touch);
        if (!tag) tag = doLuaEventTouchIn(HANDLE_TOUCHIN, this, "Base",touch->getLocation());
        if (event == TOUCH_DOWN && !this->touch && tag)
        {
            this->touch = touch;
            if (doTouch(event,touch))
            {
                doLuaEventTouchIn(HANDLE_DOWN, this, "Base",touch->getLocation());
                if (touchHandle) touchHandle->handleDown(this);
                return true;
            }
            return false;
        }
        if (event == TOUCH_MOVE && touch == this->touch)
        {
            doLuaEventTouchIn(HANDLE_MOVE, this, "Base",touch->getLocation());
            if (doTouch(event,touch))
            {
                if (touchHandle) touchHandle->handleMove(this);
                return true;
            }
            return false;
        }
        if (event == TOUCH_END && touch == this->touch)
        {
            bool tag =  doTouch(event,touch);
            this->touch = NULL;
            doLuaEventTouchIn(HANDLE_END, this, "Base",touch->getLocation());
            if (touchHandle) touchHandle->handleEnd(this);
            return tag;
        }
        return false;
    }
    void Base::show()
    {
        setVisible(true);
    }
    void Base::hide()
    {
        setVisible(false);
    }
    /////////////////////////////////////////////////////////////////////////////////////////////
    // 图像
    // 可以点击图像的有效区域 基于像素判断
    /////////////////////////////////////////////////////////////////////////////////////////////
	Image *Image::create(const char *pngName)
	{
		Image *image = new Image;
		if (image)
		{
            if (pngName)
            {
                image->setOutLine(pngName);
                image->pngName = pngName;
                image->replacePng(pngName);
            }
            image->autorelease();
		}
		return image;
	}
	/**
     * 设置轮廓图片
     */
	void Image::setOutLine(const char *pngName)
	{
		this->outlinePngName = pngName;
        if (!strncmp(pngName, "", 1)) return;
		if (image) delete image;image = NULL;
		createImage(pngName,image);
	}
	/**
	 * 检查是否命中Touch
	 */
	bool Image::checkIn(CCTouch *touch)
	{
		if (!touch) return false;
		CCPoint point = touch->getLocation();
		return checkIn(point);
	}
    void Base::addTimer(int timeid,int timeout)
    {
        if (timeout == -1)
        {
            TIMERS_ITER iter = timers.find(timeid);
            if (iter != timers.end())
            {
                timers.erase(iter);
            }
        }
        else
        {
            Timer timer;
            timer.timeid = timeid;
            timer.timeout = timeout;
            timers[timeid] = timer;
        }
    }
	void Image::replacePng(const char *name)
	{
        if (!strncmp(name, "", 1)) return;
		if (sprite)
		{
            sprite->initWithFile(name);
            
            //if (imageSize.width && imageSize.height)
            imageSize.width = sprite->getContentSize().width;
            imageSize.height = sprite->getContentSize().height;
            {
                setsize(imageSize);
            }
        }
		else
		{
			sprite = CCSprite::create(name);
			setOutLine(name);
            this->addChild(sprite);
            imageSize.width = sprite->getContentSize().width;
            imageSize.height = sprite->getContentSize().height;
            {
                setsize(imageSize);
            }

        }
	}
    void Image::clear()
    {
        for (SHAPES_ITER iter = shapes.begin(); iter != shapes.end();++iter)
        {
            if (*iter) delete *iter;
        }
        shapes.clear();
    }
    
    DrawShapeExt * Image::pushShape(DrawShapeExt *shape)
    {
        shapes.push_back(shape);
        return shape;
    }
    void Image::showShapes()
    {
        for (SHAPES_ITER iter = shapes.begin(); iter != shapes.end();++iter)
        {
            if (*iter) (*iter)->show();
        }
    }
    void Image::draw()
    {
        if (!sprite)
        {
            showShapes();
        }
        cc_timeval nowtime;
        CCTime::gettimeofdayCocos2d(&nowtime, NULL);
        for (TIMERS_ITER iter = timers.begin();iter != timers.end();++iter)
        {
            int timeout = CCTime::timersubCocos2d(&iter->second.nowTime,&nowtime) ;
            if (timeout >= iter->second.timeout)
            {
                doLuaTimerCallback(this,"Image",iter->second.timeid);
                CCTime::gettimeofdayCocos2d(&iter->second.nowTime, NULL);
            }
        }
    //    ccColor4B color = {255,255,255,255};
        ccDrawColor4F(1, 1, 1, 1);
        glLineWidth(1);
    }
	/**
     * 检查该点是否在图片上
     */
	bool Image::checkIn(const CCPoint &point)
	{
		if (!sprite) return false;
		CCPoint pos = this->convertToNodeSpace(point);
		return checkIn(pos,sprite,this->outlinePngName.c_str());
	}
	bool Image::checkIn(const CCPoint point,CCSprite *sprite,const char *fileName)
	{
		if (!sprite || !image) return false;
		if (checkInRect(point,this->sprite))
		{
            if (this->sprite->getScaleX() != 1 || this->sprite->getScaleY() != 1) return true;
			ccColor4B c = {0, 0, 0, 0};
			CCPoint touchPoint = point;
			CCSize cSize = sprite->getContentSize();
			CCPoint pos(sprite->getPositionX() - cSize.width/2, sprite->getPositionY()- cSize.height/2);
			CCPoint localPoint = ccp(touchPoint.x - pos.x,cSize.height - (touchPoint.y -pos.y));
			if (localPoint.x < 0 || localPoint.y < 0) return false;
			unsigned int x = localPoint.x, y = localPoint.y;
			unsigned char *data_ = image->getData();
			if (image->getHeight() > cSize.height || image->getWidth() > cSize.width) return false;
			unsigned int *pixel = (unsigned int *)data_;
			pixel = pixel + (y * (int)cSize.width) + x ;
			if ((y * (int)cSize.width) + x >= image->getDataLen()) return false;
			c.r = *pixel & 0xff;
			c.g = (*pixel >> 8) & 0xff;
			c.b = (*pixel >> 16) & 0xff;
			c.a = (*pixel >> 24) & 0xff;
			if (c.a == 0) {
				return false;
			}else
			{
				return true;
			}
            
		}
		else return false;
	}
	bool Image::createImage(const char *fileName,CCImage* & image)
	{
		if (!fileName) return false;
		std::string pathKey = fileName;
		image = new CCImage;
		pathKey = CCFileUtils::sharedFileUtils()->fullPathFromRelativePath(pathKey.c_str());
        
		std::string fullpath = pathKey;
		{
			std::string lowerCase(fileName);
			for (unsigned int i = 0; i < lowerCase.length(); ++i)
			{
				lowerCase[i] = tolower(lowerCase[i]);
			}
			do
			{
				CCImage::EImageFormat eImageFormat = CCImage::kFmtUnKnown;
				if (std::string::npos != lowerCase.find(".png"))
				{
					eImageFormat = CCImage::kFmtPng;
				}
				else
				{
					return false;
				}
                
				unsigned long nSize = 0;
				unsigned char* pBuffer = CCFileUtils::sharedFileUtils()->getFileData(fullpath.c_str(), "rb", &nSize);
				if (! image->initWithImageData((void*)pBuffer, nSize, eImageFormat))
				{
					CC_SAFE_DELETE_ARRAY(pBuffer);
					break;
				}
				else
				{
					CC_SAFE_DELETE_ARRAY(pBuffer);
				}
				return true;
			} while (0);
		}
		return false;
	}
	bool Image::checkInRect(const CCPoint point,CCSprite *sprite)
	{
		if (!sprite) return false;
        float width = sprite->getContentSize().width * sprite->getScaleX();
        float height = sprite->getContentSize().height * sprite->getScaleY();
		CCRect rect = CCRectMake(
                                 sprite->getPositionX()-width/2,
                                 sprite->getPositionY()-height/2,
                                 width,
                                 height);
		if (rect.containsPoint(point))
		{
			return true;
		}
		return false;
	}
    void Image::setsize(const CCSize &size)
    {
        imageSize = size;
        if (sprite)
        {
            sprite->setScaleX(size.width / sprite->getContentSize().width);
            sprite->setScaleY(size.height / sprite->getContentSize().height);
        }
    }
    void  Image::setResizeable(bool tag)
    {
        resizeable = tag;
        moveAble = false;
    }
    DrawShapeExt * Image::getShape(int index)
    {
        int i = 0;
        for (SHAPES_ITER iter = shapes.begin();iter != shapes.end();++iter)
        {
            if (i++ == index) return *iter;
        }
        return NULL;
    }
	bool Image::doTouch(TouchEvent event,CCTouch *touch)
	{
		CCPoint pos = touch->getLocation();
		switch(event)
		{
			case TOUCH_DOWN:
			{
				nowTouchPoint = pos;
				return true;
			}break;
			case TOUCH_MOVE:
			{
                if (moveAble)
                {
                    CCPoint nowPoint = this->getPosition();
                    this->setPosition(ccp(nowPoint.x + pos.x - nowTouchPoint.x,
                                          nowPoint.y + pos.y - nowTouchPoint.y));
                    nowTouchPoint = pos;
                    return true;
                }
                if (resizeable)
                {
                    CCPoint inNodePoint = this->convertToNodeSpace(pos);
                    float dw = fabsf(inNodePoint.x - 0);
                    float dh = fabsf(inNodePoint.y - 0);
                    float dx = (inNodePoint.x - getPositionX() + imageSize.width/2) /2 + getPositionX();
                    float dy = (inNodePoint.y - getPositionY() + imageSize.height/2) /2 + getPositionY();
                    setsize(CCSizeMake(dw*2, dh*2));
                   // setPosition(ccp(dx,dy));
                    return true;
                }
			}break;
			case  TOUCH_END:
			{
                
			}break;
		}
		return false;
	}
    void Image::runAction(CCAction *action)
    {
        if (sprite) sprite->runAction(action);
    }
    CCSize Image::getViewSize()
    {
        if (sprite) return sprite->getContentSize();
        return CCSizeMake(0, 0);
    }
	/**
	 * 限制元素在视图内部
	 */
	void Image::setInView(const CCSize &size)
	{
        float width = size.width;
        float height = size.height;
		if (getPositionX() >= width - getViewSize().width/2)
		{
			setPositionX(width - getViewSize().width/2);
		}
		if (getPositionX() <= getViewSize().width/2)
		{
			setPositionX(getViewSize().width/2);
		}
		if (getPositionY() >= height - getViewSize().height/2)
		{
			setPositionY(height - getViewSize().height/2);
		}
		if (getPositionY() <= getViewSize().height/2)
		{
			setPositionY(getViewSize().height/2);
		}
	}
	/**
     * 响应touch事件
     * 主要控制图片的移动
     */
	bool Button::doTouch(TouchEvent event,CCTouch *touch)
	{
		CCPoint pos = touch->getLocation();
		switch(event)
		{
			case TOUCH_DOWN:
			{
                //doLuaEventTouchIn(HANDLE_DOWN, this, "Button",touch->getLocation());
				nowTouchPoint = pos;
				this->replacePng(downName.c_str());
				up = false;
                return true;
			}break;
			case TOUCH_MOVE:
			{
                if (moveAble)
                {
                    CCPoint nowPoint = this->getPosition();
                    this->setPosition(ccp(nowPoint.x + pos.x - nowTouchPoint.x,
                                          nowPoint.y + pos.y - nowTouchPoint.y));
                    nowTouchPoint = pos;
                    return true;
                }
			}break;
			case  TOUCH_END:
			{
               // doLuaEventTouchIn(HANDLE_DOWN, this, "Button",touch->getLocation());
                up = true;
				this->replacePng(upName.c_str());
                if (Image::checkIn(touch))
                {
                    if (touchHandle)
                        touchHandle->handleClick(this);
                    doLuaEvent(HANDLE_CLICK, this, "Button");
                }
				return true;
			}break;
		}
		return false;
	}
    /////////////////////////////////////////////////////////////////////////////////////////////
    // 按钮
    // 接受事件对应的外形的变幻
    /////////////////////////////////////////////////////////////////////////////////////////////
    
	/**
	 * 创建按钮
	 */
	Button * Button::create(const char *upName,const char *downName)
	{
		Button *button = new Button();
		if (button)
		{
            if (upName && downName)
            {
                button->upName = upName;
                button->downName=downName;
                button->replacePng(upName);
            }
            button->autorelease();
		}
		return button;
	}
    
    void Button::draw()
    {
        if (sprite) return;
        if (doLuaEvent(HANDLE_DRAW, this, "Button") == 0)
        {
            showShapes();
            ccDrawColor4F(1, 1, 1, 1);
           glLineWidth(1);

            return;
            float width = imageSize.width;
            float height = imageSize.height;
            if (!width || !height)
            {
                width = 96;
                height = 32;
            }
            CCPoint src = ccp(-width/2,-height/2);
            CCPoint dest = ccpAdd(src, ccp(width,height));
            if (up)
            {
                ccColor4F color = {0.5,0.5,1,1};
                ccDrawSolidRect(src, dest, color);
            }
            else
            {
                ccColor4F color = {0,0.5,1,1};
                ccDrawSolidRect(src, dest, color);
            }
        }
        else
        {
            showShapes();
            ccDrawColor4F(1, 1, 1, 1);
            glLineWidth(1);
        }

    }
    /**
     * 检查该点是否在图片上
     */
	bool Button::checkIn(const CCPoint &point)
	{
        if (sprite) return Image::checkIn(point);
        CCPoint pos = this->convertToNodeSpace(point);

        for (SHAPES_ITER iter = shapes.begin(); iter != shapes.end();++iter)
        {
            if ((*iter)->checkIn(pos))
            {
                return true;
            }
        }
        bool tag = doLuaEvent(HANDLE_TOUCHIN, this, "Button") ;
        if (tag != 0) {
            if (tag == 1) {
                return true;
            }
            return false;
        }
        float width = imageSize.width;
        float height = imageSize.height;
        if (!width || !height)
        {
            width = 96;
            height = 32;
        }
		CCRect rect = CCRectMake(
                                 -width/2,
                                 -height/2,
                                 width,
                                 height);
		if (rect.containsPoint(pos))
		{
			return true;
		}
		return false;
	}
    
    /**
     * 响应touch事件
     * 主要控制图片的移动
     */
	bool Choice::doTouch(TouchEvent event,CCTouch *touch)
	{
		CCPoint pos = touch->getLocation();
		switch(event)
		{
			case TOUCH_DOWN:
			{
				nowTouchPoint = pos;
				return true;
			}break;
			case TOUCH_MOVE:
			{
                if (moveAble)
                {
                    CCPoint nowPoint = this->getPosition();
                    this->setPosition(ccp(nowPoint.x + pos.x - nowTouchPoint.x,
                                          nowPoint.y + pos.y - nowTouchPoint.y));
                    nowTouchPoint = pos;
                    return true;
                }
			}break;
			case  TOUCH_END:
			{
                if (!choiceAble)
                {
                    this->replacePng(downName.c_str());
                }
				else
                    this->replacePng(upName.c_str());
				choiceAble = !choiceAble;
                if (Image::checkIn(touch))
                {
                    if (touchHandle)
                        touchHandle->handleClick(this);
                    doLuaEvent(HANDLE_CLICK, this, "Choice");
                }
                return true;
			}break;
		}
		return false;
	}
    bool Choice::checkIn(const cocos2d::CCPoint &point)
    {
        if (sprite) return Image::checkIn(point);
        float width = imageSize.width;
        float height = imageSize.height;
        if (!width || !height)
        {
            width = 32;
            height = 32;
        }
		CCPoint pos = this->convertToNodeSpace(point);
		CCRect rect = CCRectMake(
                                 -width/2,
                                 -height/2,
                                 width,
                                 height);
		if (rect.containsPoint(pos))
		{
			return true;
		}
		return false;
        
    }
    void Choice::draw()
    {
        if (sprite) return;
        if (doLuaEvent(HANDLE_DRAW, this, "Choice"))return;
        float width = imageSize.width;
        float height = imageSize.height;
        if (!width || !height)
        {
            width = 32;
            height = 32;
        }
        CCPoint src = ccp(-width/2,-height/2);
        CCPoint dest = ccpAdd(src, ccp(width,height));
        //ccColor4F color = {0.5,0.5,1,1};
        ccDrawRect(src, dest);
        
        // 绘制区域
        if (choiceAble)
        {
            // 绘制钩子
            glLineWidth(3);
            CCPoint start = ccp(-width/2,height/2);
            CCPoint mid = ccp(-width/2 + 8,-height/2);
            CCPoint end = ccp(width/2,height/2);
            ccDrawLine(start, mid);
            ccDrawLine(mid, end);
        }
        
        glLineWidth(1);
        ccDrawColor4B(255, 255, 255, 255);
        ccPointSize(1);
    }
	/**
	 * 创建选择器
	 */
	Choice * Choice::create(const char *upName,const char *downName)
	{
		Choice *choice = new Choice();
		if (choice)
		{
            if (upName && downName)
            {
                choice->upName = upName;
                choice->downName=downName;
                choice->replacePng(upName);
            }
            choice->choiceAble = false;
            choice->autorelease();
		}
		return choice;
	}
    /**
     *响应touch事件
     * 主要控制图片的移动
     */
	bool Bubble::doTouch(TouchEvent event,CCTouch *touch)
	{
		CCPoint pos = touch->getLocation();
		switch(event)
		{
			case TOUCH_DOWN:
			{
				nowTouchPoint = pos;
				return true;
			}break;
			case TOUCH_MOVE:
			{
                if (moveAble)
                {
                    CCPoint nowPoint = this->getPosition();
                    this->setPosition(ccp(nowPoint.x + pos.x - nowTouchPoint.x,
                                          nowPoint.y + pos.y - nowTouchPoint.y));
                    nowTouchPoint = pos;
                    return true;
                }
			}break;
			case  TOUCH_END:
			{
                if (!choiceAble)
                {
                    this->replacePng(downName.c_str());
                }
				choiceAble = true;
                if (Image::checkIn(touch))
                {
                    if (touchHandle)
                        touchHandle->handleClick(this);
                    doLuaEvent(HANDLE_CLICK, this, "Bubble");
                }
                return true;
			}break;
		}
		return false;
	}
    void Bubble::popUp()
    {
        choiceAble = false;
        this->replacePng(upName.c_str());
    }
    
    void Bubble::draw()
    {
        if (sprite) return;
        if (doLuaEvent(HANDLE_DRAW, this, "Bubble"))return;
        float width = imageSize.width;
        float height = imageSize.height;
        if (!width || !height)
        {
            width = 96;
            height = 32;
        }
        CCPoint src = ccp(-width/2,-height/2);
        CCPoint dest = ccpAdd(src, ccp(width,height));
        if (choiceAble)
        {
            ccColor4F color = {0.5,0.5,1,1};
            ccDrawSolidRect(src, dest, color);
        }
        else
        {
            ccColor4F color = {0,0.5,1,1};
            ccDrawSolidRect(src, dest, color);
        }
        
    }
    bool Bubble::checkIn(const cocos2d::CCPoint &point)
    {
        if (sprite) return Image::checkIn(point);
        float width = imageSize.width;
        float height = imageSize.height;
        if (!width || !height)
        {
            width = 96;
            height = 32;
        }
		CCPoint pos = this->convertToNodeSpace(point);
		CCRect rect = CCRectMake(
                                 -width/2,
                                 -height/2,
                                 width,
                                 height);
		if (rect.containsPoint(pos))
		{
			return true;
		}
		return false;
        
    }
    
	/**
	 * 创建选择器
	 */
	Bubble * Bubble::create(const char *upName,const char *downName)
	{
		Bubble *bubble = new Bubble();
		if (bubble)
		{
            if (upName && downName)
            {
                bubble->upName = upName;
                bubble->downName=downName;
                bubble->replacePng(upName);
            }
            bubble->choiceAble = false;
            bubble->autorelease();
		}
		return bubble;
	}
    /////////////////////////////////////////////////////////////////////////////////////////////
    // 文本信息
    // 可以显示 文字 和 图像
    /////////////////////////////////////////////////////////////////////////////////////////////
    
    /**
     * 创建Label
     */
    Label * Label::create(const char *content,const CCSize& size)
    {
        Label *label = new Label;
        if (label->init(content, size))
        {
            label->autorelease();
            return label;
        }
        CC_SAFE_DELETE(label);
        return NULL;
    }
    void Label::setNumber(int tag,double value)
    {
        if (tag >= 0 && tag <= numbers.size())
        {
            CCLabelAtlas * number = numbers[tag];
            if (number)
            {
                char buffer[256] = {'\0'};
                sprintf(buffer, "%g",value);
                number->setString(buffer);
            }
        }
    }
    /**
     * 解析文本在区域内展示内容
     */
    bool Label::init(const char* content,const CCSize &size)
    {
		rect.origin = ccp(0,0);
		rect.size = size;
        float x = 0, y = 0;
        numbers.clear();
        while (*content && *content != '\0')
        {
            if(*content == 'I') // I (test.png&100&100)
            {
                std::vector<std::string> args;
                content = getArgs(content,3,args);
                if (args.size() == 3)
                {
                    CCSprite * temp = CCSprite::create(args[0].c_str());
                    if (temp)
                    {
                        temp->setAnchorPoint(ccp(0,0));
                        temp->setScaleX(atoi(args[1].c_str()) / temp->getContentSize().width);
                        temp->setScaleY(atoi(args[2].c_str()) / temp->getContentSize().height);
                        this->addChild(temp);
                        temp->setPosition(ccp(x,y));
                        x+= temp->getContentSize().width * temp->getScaleX();
                    }
                }
            }
            if (*content == 'T')// T(hello,world & Arial &12)
            {
                std::vector<std::string> args;
                content = getArgs(content,6,args);
                if (args.size() == 3 || args.size() == 6)
                {
                    CCLabelTTF *temp = CCLabelTTF::create(args[0].c_str(), args[1].c_str(), atoi(args[2].c_str())/*,CCSizeMake(200,20),kCCTextAlignmentLeft*/);
                    //temp->setAnchorPoint(ccp(0,0));
                    if (temp)
                    {
                        this->addChild(temp);
                        temp->setPosition(ccp(x,y));
                        x += temp->getContentSize().width;
                    }
                    if (args.size() == 6)
                    {
                        temp->setColor(ccc3(atoi(args[3].c_str()), atoi(args[4].c_str()), atoi(args[5].c_str())));
                    }
                }
            }
            if (*content == 'N') // number
            {
                std::vector<std::string> args;
                content = getArgs(content, 8, args);
                if (args.size() == 5 || args.size() == 8)
                {
                    CCLabelAtlas * nL= CCLabelAtlas::create(args[0].c_str(),args[1].c_str(), atoi(args[2].c_str()), atoi(args[3].c_str()),args[4].size() ? args[4][0]:'.');
                    if (nL)
                    {
                        nL->setAnchorPoint(ccp(0, 0));
                        this->addChild(nL);
                        nL->setPosition(ccp(x,y));
                        x += nL->getContentSize().width;
                        numbers.push_back(nL);
                    }
                    if (args.size() == 8 && nL)
                    {
                        nL->setColor(ccc3(atoi(args[4].c_str()), atoi(args[5].c_str()), atoi(args[6].c_str())));
                    }

                }
            }
            if (*content == 'L') // 换行
            {
                std::vector<std::string> args;
                content = getArgs(content,1,args);
                if (args.size() == 1)
                {
                    y -= atoi(args[0].c_str());
                    x = 0;
                }
            }
            if (*content == 'S') // 空格
            {
                std::vector<std::string> args;
                content = getArgs(content,1,args);
                if (args.size() == 1)
                {
                    x += atoi(args[0].c_str());
                }
            }
            content++;
        }
        return true;
    }
    const char* Label::getArgs(const char *content,int argc,std::vector<std::string> &args)
    {
        while(*content != '(') content++;
        content++;
        for (int i = 0; i < argc -1;i++)
        {
            std::string arg;
            while (*content != '&' && *content != '\0' && *content != ')')
            {
                arg.push_back(*content);
                content++;
            }
            content++;
            //arg.erase(0, arg.find_first_not_of(" /t/n/r")).erase(arg.find_last_not_of(" /t/n/r") + 1);
            args.push_back(arg);
        }
        if (*content == ')') return content;
        std::string arg;
        while (*content != ')' && *content != '\0')
        {
            arg.push_back(*content);
            content++;
        }
       // arg.erase(0, arg.find_first_not_of(" /t/n")).erase(arg.find_last_not_of(" /t/n") + 1);
        args.push_back(arg);
        return content;
    }
    /**
     * 检查是否命中Touch
     */
    bool Label::checkIn(CCTouch *touch)
    {
        if (!touch) return false;
		CCPoint point = touch->getLocation();
        point = this->convertToNodeSpace(point);
        return rect.containsPoint(point);
    }
    
    void Label::draw()
    {
        if (doLuaEvent(HANDLE_DRAW, this, "Label"))return;
       // CCPoint src = ccp(0,0);
       // CCPoint dect = ccpAdd(src, ccp(rect.size.width,rect.size.height));
       // ccDrawRect(src, dect);
        cc_timeval nowtime;
        CCTime::gettimeofdayCocos2d(&nowtime, NULL);
        for (TIMERS_ITER iter = timers.begin();iter != timers.end();++iter)
        {
            int timeout = CCTime::timersubCocos2d(&iter->second.nowTime,&nowtime) ;
            if (timeout >= iter->second.timeout)
            {
                doLuaTimerCallback(this,"Label",iter->second.timeid);
                CCTime::gettimeofdayCocos2d(&iter->second.nowTime, NULL);
            }
        }

    }
    /**
     * 响应touch事件
     * 主要控制图片的移动
     */
    bool Label::doTouch(TouchEvent event,CCTouch *touch)
    {
        CCPoint pos = touch->getLocation();
		switch(event)
		{
            case TOUCH_DOWN:
			{
				nowTouchPoint = pos;
				return true;
			}break;
			case TOUCH_MOVE:
			{
                if (moveAble)
                {
                    CCPoint nowPoint = this->getPosition();
                    this->setPosition(ccp(nowPoint.x + pos.x - nowTouchPoint.x,
                                          nowPoint.y + pos.y - nowTouchPoint.y));
                    nowTouchPoint = pos;
                    return true;
                }
			}break;
            default:
                break;
		}
		return false;
    }
    double Label::getNumber(int tag)
    {
        if (tag >= 0 && tag <= numbers.size())
        {
            CCLabelAtlas * number = numbers[tag];
            if (number)
            {
                return atof(number->getString());
            }
        }
        return 0;
    }
    /**
     * 创建树
     **/
    Tree * Tree::create(Image *image)
    {
        Tree * tree = new Tree;
        tree->image = image;
        tree->image->moveAble = false;
        tree->addChild(image);
        tree->autorelease();
        return tree;
    }
    /**
     * 增加树
     */
    void Tree::addTree(Tree *tree)
    {
        if (!tree) return;
        this->addChild(tree);
        tree->parent = this;
        Tree *pre = this->child;
        if (!pre)
        {
            this->child = tree;
            show();
            return;
        }
        Tree *temp = this->child->next;
        while (temp)
        {
            pre = temp;
            temp = temp->next;
        }
        pre->next = tree;
        show();
    }
    /////////////////////////////////////////////////////////////////////////////////////////////
    // 创建树形结构
    /////////////////////////////////////////////////////////////////////////////////////////////
    
    /**
     * 检查是否命中Touch
     */
    bool Tree::checkIn(CCTouch *touch)
    {
        if (!touch) return false;
        return checkInTree(touch,this);
    }
    /**
     * 节点是否命中
     */
    bool Tree::checkInTree(CCTouch *touch,Tree *tree)
    {
        if (!touch || !tree) return false;
        if (tree->image->checkIn(touch)) return true;
        if (tree->isOpen && tree->child->checkInTree(touch, tree->child)) return true;
        Tree *node = tree->next;
        while (node) {
            if (checkInTree(touch,node)) return true;
            node = node->next;
        }
        return false;
    }
    /**
     * 响应touch事件
     * 主要控制图片的移动
     */
    bool Tree::doTouch(TouchEvent event,CCTouch *touch)
    {
        if (!touch) return false;
        return touchInTree(event,touch,this);
    }
    /**
     * 点击树节点
     */
    bool Tree::touchInTree(TouchEvent event,CCTouch *touch,Tree *tree)
    {
        if (!tree) return false;
        if (tree->image->attachTouch(event, touch))
        {
            if (checkIn(touch))
            {
                if (touchHandle)
                    touchHandle->handleClick(tree);
                doLuaEvent(HANDLE_CLICK, this, "Tree");
            }
            return true;
        }
        if (tree->isOpen && tree->child->touchInTree(event, touch, tree->child)) return true;
        Tree *node = tree->next;
        while (node) {
            if (touchInTree(event,touch,node)) return true;
            node = node->next;
        }
        return false;
    }
    /**
     * 展示这颗树
     */
    void Tree::show()
    {
        if (!image) return;
        show(this,ccp(0,0));
    }
    /**
     * 展示树结构
     */
    CCPoint Tree::show(Tree *tree,const CCPoint &point)
    {
        if (!tree || !tree->image) return ccp(0,0);
        tree->setPosition(point);
        float dy = tree->image->getViewSize().height;
        Tree * node = tree->child;
        if (node && tree->isOpen)
        {
            CCPoint offset = point;
            offset.x += tree->image->getViewSize().width;
            offset.y -= tree->image->getViewSize().height;
            CCPoint point = show(node,offset);
            dy -= point.y;
        }
        node = tree->next;
        while (node) {
            CCPoint point = show(node,ccp(point.x,dy));
            dy -= point.y;
            node = node->next;
        }
        return ccp(point.x,dy);
    }
    /**
     * 隐藏关闭节点
     **/
    void Tree::hideCloseNode(Tree *node)
    {
        if (!node) return;
        if (node->child)
        {
            if (!node->isOpen) node->child->setVisible(false);
            else node->child->setVisible(true);
            
            hideCloseNode(node->child);
        }
        Tree * temp = next;
        while (temp) {
            hideCloseNode(temp);
            temp = temp->next;
        }
        
    }
    /**
     * 处理树的渲染
     */
    void Tree::visit()
    {
        hideCloseNode(this);
        CCNode::visit();
    }
    /////////////////////////////////////////////////////////////////////////////////////////////
    // List 提供无限控件的布局管理 无限向下排布
    /////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * 创建列表
     */
    List *List::create(Image *image,const CCSize &cellSize)
    {
        List *list = new List;
        list->cellSize = cellSize;
        list->image = image;
        image->setsize(cellSize);
        list->content.push_back(image);
        list->addChild(image);
        list->autorelease();
        return list;
    }
    /**
     * 删除列表
     */
    void List::remove(List *list)
    {
        
    }
    /**
     * 增加列表
     */
    void List::add(Image *image)
    {
        addChild(image);
        content.push_back(image);
        image->setsize(cellSize);
        show();
    }
    /**
     * 展示列表
     */
    void List::show()
    {
        CCPoint offset;
        for (std::list<Image*>::iterator iter = content.begin();iter != content.end();++iter)
        {
            Image *image = *iter;
            if (image)
            {
                image->setPosition(ccp(offset.x + cellSize.width/2,offset.y + cellSize.height/2));
                offset.y -= cellSize.height;
            }
        }
    }
    /**
     * 检查是否命中Touch
     */
    bool List::checkIn(CCTouch *touch)
    {
        if (!touch) return false;
        int index = pickImageIndex(touch);
        if (index >= 0 && index <= content.size())
            return true;
        return false;
    }
    /**
     * 响应touch事件
     * 主要控制图片的移动
     */
    bool List::doTouch(TouchEvent event,CCTouch *touch)
    {
        switch (event) {
            case TOUCH_DOWN:
            {
                nowTouchImage = pickImage(pickImageIndex(touch));
                if (nowTouchImage)
                {
                    nowTouchImage->doTouch(event, touch);
                    return true;
                }
                return false;
            }
                break;
            case TOUCH_MOVE:
            {
                if (nowTouchImage)
                {
                    // 移动image
                    nowTouchImage->doTouch(event, touch);
                }
            }break;
            case TOUCH_END:
            {
                if (nowTouchImage)
                {
                    nowTouchImage->doTouch(event, touch);
                    if (exchageAble)
                    {
                        Image *nowImage = pickImage(pickImageIndex(touch));
                        if (nowImage)
                        {
                            // doExchange(nowTouchImage,nowImage);
                        }
                    }
                    show();
                }
            }break;
            default:
                break;
        }
        return false;
    }
    /**
     * 获取改点对应的Table中的引索
     */
    int List::pickImageIndex(CCTouch *touch)
    {
        if (!touch) return -1;
        if (content.empty()) return -1;
        CCPoint point = this->convertToNodeSpace(touch->getLocation());
        if (point.x >=0 && point.y >=0 && point.x <= cellSize.width && point.y <= cellSize.height)
        {
            return 0;
        }
        if (content.size() < 2) return -1;
        int contentSize = content.size() - 1;
        if (point.x >= 0 && point.x <= cellSize.width && point.y <=0)
        {
            int index = abs(point.y) / cellSize.height;
            if (index < contentSize)
                return index + 1;
        }
        return -1;
    }
    
    /**
     * 根据引索获取图像
     */
    Image *List::pickImage(const int &index)
    {
        if (index < content.size() && index >= 0)
        {
            std::list<Image*>::iterator begin = content.begin();
            int count = 0;
            while (begin != content.end() && count < index) {
                begin++;
                count++;
            }
            if (begin != content.end())
            {
                return *begin;
            }
        }
        return NULL;
    }
	/////////////////////////////////////////////////////////////////////////////////////////////
	// 滑块
	/////////////////////////////////////////////////////////////////////////////////////////////
    /**
	 * 创建滑块
	 */
	Slider * Slider::create(const char *upName,const char *downName)
	{
		Slider *slider = new Slider;
		slider->init(upName,downName);
		slider->autorelease();
		return slider;
	}
	
	/**
	 * 初始化滑块
	 */
	bool Slider::init(const char *upName,const char *downName)
	{
		if (upName && downName)
		{
			sliderSprite = CCSprite::create(upName);
			backSprite = CCSprite::create(downName);
			backSprite->setAnchorPoint(ccp(0,0));
			this->addChild(sliderSprite);
			this->addChild(backSprite);
			setValue(0);
			sliderSprite->setPositionY(backSprite->getContentSize().height/2);
			return true;
		}
        
		return false;
	}
    
	/**
     * 检查是否命中Touch
     */
    bool Slider::checkIn(CCTouch *touch)
	{
		if (!touch) return false;
		CCPoint point = this->convertToNodeSpace(touch->getLocation());
		if (point.x >= 0 && point.x <= backSprite->getContentSize().width && point.y >= 0 && point.y <= backSprite->getContentSize().height)
		{
			return true;
		}
		return false;
	}
    /**
     * 响应touch事件
     * 主要控制图片的移动
     */
    bool Slider::doTouch(TouchEvent event,CCTouch *touch)
	{
		CCPoint point = this->convertToNodeSpace(touch->getLocation());
		switch (event) {
            case TOUCH_DOWN:
            {
				float value = (point.x - sliderSprite->getContentSize().width) / getMaxValue();
				this->setValue(value);
                return true;
            }
                break;
            case TOUCH_MOVE:
            {
				float value = (point.x - sliderSprite->getContentSize().width) / getMaxValue();
				this->setValue(value);
            }break;
            case TOUCH_END:
            {
                
                
            }break;
            default:
                break;
        }
		return false;
	}
	
	/**
	 * 获取值
	 */
	float Slider::getValue()
	{
		return value;
	}
	/**
	 * 设置当前示意值
	 */
	void Slider::setValue(float value)
	{
		if (value < 0) value = 0;
		if (value > 1) value = 1;
		this->value = value;
		if (sliderSprite)
		{
			float x = value * getMaxValue() + sliderSprite->getContentSize().width/2;
            
			sliderSprite->setPositionX(x);
		}
	}
	float Slider::getMaxValue()
	{
		return (backSprite->getContentSize().width - sliderSprite->getContentSize().width);
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
    // Table 提供有限控件的布局管理 可以交换格子上的内容
    /////////////////////////////////////////////////////////////////////////////////////////////
    Table *Table::create(const CCSize & tableSize,const CCSize &cellSize)
    {
        Table *table = new Table;
        table->autorelease();
        table->tableSize = tableSize;
        table->images.resize(tableSize.width * tableSize.height);
        table->cellSize = cellSize;
        //Image * test = Image::create("Icon.png");
        //table->addChild(test);
        return table;
    }
    /**
     * 增加一个元素
     */
    void Table::add(const CCPoint &point,Image *image)
    {
        if (point.x >= 0 && point.y >=0 && point.x < tableSize.width && point.y < tableSize.height)
        {
            images[point.y * tableSize.width + point.x] = image;
            image->setsize(CCSizeMake(cellSize.width, cellSize.height));
            image->retain();
            //this->addChild(image);
            image->moveAble = true;
        }
        show();
    }
    /**
     * 删除一个元素
     */
    void Table::remove(Image *image)
    {
        for (int i = 0; i < images.size(); ++i) {
            if (image == images[i])
            {
				image->getParent()->removeChild(image,true);
                images[i] = NULL;
                break;
            }
        }
    }
    void Table::removeIndex(Image *src)
    {
        for (int i = 0; i < images.size(); ++i) {
            if (src == images[i])
            {
                images[i] = NULL;
                break;
            }
        }
    }
    /**
     * 检查是否命中Touch
     */
    bool Table::checkIn(CCTouch *touch)
    {
        CCPoint gridIndex = pickImageIndex(touch);
        int index = gridIndex.x + gridIndex.y * tableSize.width;
        if (index >= 0 && index < images.size())
            return true;
        return false;
    }
    /**
     * 按次序增加一个元素
     */
    void Table::push(Image *image)
    {
        if (!image) return;
        for (int j = tableSize.height; j >= 0;j--)
        {
            bool tag = false;
            for (int i = 0; i <= tableSize.width; i++)
            {
                int index = i + j * tableSize.width;
                if (index >=0 && index < images.size() && !images[index])
                {
                    images[index] = image;
                    image->setsize(CCSizeMake(cellSize.width-4, cellSize.height-4));
                    image->retain();
                    image->moveAble = true;
                    tag = true;
                    break;
                }
            }
            if (tag) break;
        }
        //this->addChild(image);
        show();
    }
    
    /**
     * 响应touch事件
     * 主要控制图片的移动
     */
    bool Table::doTouch(TouchEvent event,CCTouch *touch)
    {
        switch (event) {
            case TOUCH_DOWN:
            {
                nowTouchImage = pickImage(pickImageIndex(touch));
                if (nowTouchImage)
                {
                    nowTouchImage->doTouch(event, touch);
                    return true;
                }
                return false;
            }
                break;
            case TOUCH_MOVE:
            {
                if (nowTouchImage)
                {
                    // 移动image
                    nowTouchImage->doTouch(event, touch);
                    return true;
                }
            }break;
            case TOUCH_END:
            {
                if (nowTouchImage)
                {
                    nowTouchImage->doTouch(event, touch);
                    if (exchageAble)
                    {
                        Image *nowImage = pickImage(pickImageIndex(touch));
                        if (nowImage)
                        {
                            doExchange(nowTouchImage,nowImage);
                        }
                        else
                        {
                            CCPoint gridIndex = pickImageIndex(touch);
                            int index = gridIndex.x + gridIndex.y * tableSize.width;
                            if (index >=0 && index < images.size())
                            {
                                removeIndex(nowTouchImage);
                                images[index] = nowTouchImage;
                            }
                        }
                    }
                    show();
                    return true;
                }
            }break;
            default:
                break;
        }
        return false;
    }
    /**
     * 低效的替换算法
     */
    void Table::doExchange(Image *src,Image *dest)
    {
        for (int i = 0;i < images.size();++i)
        {
            Image *image = images[i];
            if (image == src)
            {
                images[i] = dest;
            }
            if (image == dest)
            {
                images[i] = src;
            }
        }
    }
    /**
     * 获取改点对应的Table中的引索
     */
    CCPoint Table::pickImageIndex(CCTouch *touch)
    {
        CCPoint point = this->convertToNodeSpace(touch->getLocation());
        if (point.x >= 0 && point.y >= 0 && point.x <= cellSize.width * tableSize.width && point.y <= cellSize.width * tableSize.height)
        {
            int x = (int)point.x / (int)cellSize.width;
            int y = (int)point.y / (int) cellSize.height;
            return ccp(x,y);
        }
        return ccp(-1,-1);
    }
    
    /**
     * 根据引索获取图像
     */
    Image *Table::pickImage(const CCPoint &point)
    {
        int index = point.x + point.y * tableSize.width;
        if (index < images.size())
            return images[index];
        return NULL;
    }
    
    /**
     * 展示元素
     */
    void Table::show()
    {
        for (int i = 0; i < images.size();i++)
        {
            int y = i / tableSize.width;
            int x = i % (int)tableSize.width;
            Image *image = images[i];
            if (image)
            {
                image->setPosition(ccp(x * cellSize.width + cellSize.width/2,y * cellSize.height + cellSize.height/2));
            }
        }
    }
    
    void Table::draw()
    {
        if (doLuaEvent(HANDLE_DRAW, this, "Table"))return;
        CCPoint src = ccp(0,0);
        CCPoint dest = ccp(tableSize.width * cellSize.width,tableSize.height*cellSize.height);
        ccColor4F color = {0,0,0,1};
        ccDrawSolidRect(src, dest, color);
        // TODO 划线
        for (unsigned int index = 0; index <= tableSize.width; index++) {
            CCPoint src = ccp(0 + index * cellSize.width,0);
            CCPoint dest = ccp(0 + index * cellSize.width,0 + tableSize.height*cellSize.height);
            ccDrawLine(src, dest);
        }
        for (unsigned int index = 0; index <= tableSize.height; index++) {
            CCPoint src = ccp(0,0+ index * cellSize.height);
            CCPoint dest = ccp(0 + tableSize.width*cellSize.width,0+ index *cellSize.height);
            ccDrawLine(src, dest);
        }
        
    }
    void Table::visit()
    {
        kmGLPushMatrix();
        this->transform();
        this->draw();
        for (int i = 0; i < images.size();i++)
        {
            Image *image = images[i];
            if (image != this->nowTouchImage && image)
            {
                image->visit();
            }
        }
        if (this->nowTouchImage)
            this->nowTouchImage->visit();
        kmGLPopMatrix();
    }
    /////////////////////////////////////////////////////////////////////////////////////////////
    // HPLabel 血量条
    /////////////////////////////////////////////////////////////////////////////////////////////
    HPLabel *HPLabel::create(const char *backName,const char *showName)
    {
        HPLabel *label = new HPLabel();
        if (label && label->init(backName, showName))
        {
            label->autorelease();
            return label;
        }
        CC_SAFE_DELETE(label);
        return NULL;
    }
    bool HPLabel::init(const char *backName,const char *showName)
    {
        this->maxValue = 100;
        this->value = 0;
        if (backName && showName)
        {
            this->backSprite = CCSprite::create(backName);
            this->backSprite->setAnchorPoint(ccp(0,0.5));
            this->addChild(this->backSprite);
            this->valueSprite = CCSprite::create(showName);
            this->valueSprite->setAnchorPoint(ccp(0,0.5));
            this->addChild(this->valueSprite);
            viewSize = this->valueSprite->getContentSize();
        }
        setValue(value);
        return true;
    }
    void HPLabel::setValue(float valuep)
    {
        this->value = valuep * maxValue;
        float width = viewSize.width;
        width *= ((float)value) / maxValue;
        if (valueSprite)
            valueSprite->setTextureRect(CCRectMake(0,0,
                                                   width,valueSprite->getContentSize().height));
    }
    
    void HPLabel::draw()
    {
        if (valueSprite) return;
        if (doLuaEvent(HANDLE_DRAW, this, "Table"))return;
        float width = maxValue;
        float height = 20;
        CCPoint src = ccp(-width/2,-height/2);
        CCPoint dest = ccpAdd(src,ccp(width,height));
        ccColor4F red = {1.0,0,0,1};
        ccDrawRect(src, dest);
        ccColor4F green = {0.2,0.4,0.6,1};
        src = ccp(-width/2 + 1,-height/2 + 1);
        dest = ccpAdd(src,ccp(value,height-4));
        ccDrawSolidRect(src, dest, green);
    }
    /////////////////////////////////////////////////////////////////////////////////////////////
    // 指定可视区域
    /////////////////////////////////////////////////////////////////////////////////////////////
    ScrollView * ScrollView::create(float width,float height)
    {
        ScrollView * view = new ScrollView;
        if (view && view->init(width,height))
        {
            view->autorelease();
            return view;
        }
        CC_SAFE_DELETE(view);
        return NULL;
    }
    bool ScrollView::init(float width,float height)
    {
        this->width = width;
        this->height = height;
        // this->setColor(ccc3(23,25,244));
        return true;
    }
    
    void ScrollView::visit()
    {
        
        this->draw();
        CCSize szLimitSize= CCSizeMake(width ,height);
        
        CCPoint worldPt = this->getPosition();// ccp(_x,_y);
        worldPt = this->getParent()->convertToWorldSpace(worldPt);
        CCSize size;
        
        if (CC_CONTENT_SCALE_FACTOR() != 1)
        {
            size.height = szLimitSize.height * CC_CONTENT_SCALE_FACTOR();
            size.width = szLimitSize.width * CC_CONTENT_SCALE_FACTOR();
            worldPt = ccpMult(worldPt, CC_CONTENT_SCALE_FACTOR());
        }
        else
            
        {
            size.width = szLimitSize.width;
            size.height =szLimitSize.height;
        }
        glEnable(GL_SCISSOR_TEST);
        CCDirector::sharedDirector()->getOpenGLView()->setScissorInPoints(worldPt.x ,worldPt.y,  size.width, size.height);
        CCNode::visit();
        glDisable(GL_SCISSOR_TEST);
    }
    void ScrollView::add(Image *base)
    {
        this->addChild(base);
        child = base;
        // 设置在左上角
        float x = base->getViewSize().width/2;
        float y = height - base->getViewSize().height/2;
        base->setPosition(ccp(x,y));
    }
    /**
     * 检查是否命中Touch
     */
    bool ScrollView::checkIn(CCTouch *touch)
    {
        CCPoint point = touch->getLocation();
        CCPoint pos = getPosition();
        pos = this->getParent()->convertToWorldSpace(pos);
        CCRect rect = CCRectMake(
                                 pos.x,
                                 pos.y,
                                 width,
                                 height);
        if (rect.containsPoint(point))
        {
            return true;
        }
        return false;
    }
    void ScrollView::draw()
    {
        CCPoint src = getPosition();
        CCPoint dest = ccpAdd(src,ccp(width,height));
        ccColor4F color = {0.2,0.3,0.1,1};
        ccDrawSolidRect(src, dest, color);
    }
    /**
     * 响应touch事件
     * 主要控制图片的移动
     */
    bool ScrollView::doTouch(TouchEvent event,CCTouch *touch)
    {
        // 处理点击事件 若移动的话 根据子类的大小 设定可以移动的范围
        if (!isVisible()) return false;
        switch (event) {
            case TOUCH_DOWN:
            {
                if (child) child->attachTouch(event, touch);
                nowTouchPoint = touch->getLocation();
                return true;
            }break;
            case TOUCH_MOVE:
            {
                CCPoint pos = touch->getLocation();
                CCPoint nowPoint = child->getPosition();
                float x = nowPoint.x + pos.x - nowTouchPoint.x;
                float y = nowPoint.y + pos.y - nowTouchPoint.y;
                if (scrollType == UP_DOWN)
                {
                    x = nowPoint.x;
                }
                if (scrollType == LEFT_RIGHT)
                {
                    y = nowPoint.y;
                }
                if (child)
                {
                    child->setPosition(ccp(x,y));
                }
                nowTouchPoint = pos;
            }break;
            case TOUCH_END:
            {
                if (child)
                {
                    child->attachTouch(event, touch);
                    // 将位置校验到eachTap 处
                    // 当位置在小于eachTap 的1/2时 对齐到0 当大于1/2时 对齐到下一个
                    if (eachTap.width)
                    {
                        float x = ((int)(child->getPositionX() + eachTap.width/2) / (int)eachTap.width) * (int)eachTap.width;
                        child->setPositionX(x);
                    }
                    if (eachTap.height)
                    {
                        float y = ((int)(child->getPositionY() + eachTap.height/2) / (int)eachTap.height) * (int)eachTap.height;
                        child->setPositionY(y);
                    }
                    
                    // 处理控件的位置信息
					child->setInView(CCSizeMake(width,height));
                }
            }break;
            default:
                break;
        }
        return false;
    }
    /////////////////////////////////////////////////////////////////////////////////////////////
    // Panel 放置子控件在其上
    /////////////////////////////////////////////////////////////////////////////////////////////
    
    Panel *Panel::create()
    {
        Panel *panel = new Panel();
        //panel->setResizeable(true);
        panel->autorelease();
        return panel;
    }
	bool Panel::attachTouch(TouchEvent event,CCTouch *touch)
	{
        if (!this->isVisible()) return false;
		if (firsts.size()) // 优先级的列表
		{
			return firsts.back()->attachTouch(event,touch);
		}
		// 设定actives
		bool tag = false;
		switch (event)
		{
			case TOUCH_DOWN:
			{
				for (BASES_ITER iter = uis.begin();iter != uis.end();++iter)
				{
					Base *base = *iter;
					if (base && base->attachTouch(event,touch))
					{
						actives.push_back(base);
						tag  = true;
					}
				}
			}break;
			case TOUCH_MOVE:
			{
				for (BASES_ITER iter = actives.begin();iter != actives.end();++iter)
				{
					Base *base = *iter;
					if (base && base->attachTouch(event,touch))
						tag = true;
				}
                
			}break;
            case TOUCH_END:
            {
                for (BASES_ITER iter = uis.begin();iter != uis.end();++iter)
				{
					Base *base = *iter;
					if (base && base->attachTouch(event,touch))
						tag = true;
				}
                
            }break;
		}
		if (event == TOUCH_END)
		{
			actives.clear();
		}
		if (!tag)
		{
			// 处理Panel 的移动
			tag = Image::attachTouch(event,touch);
		}
		return tag;
	}
	void Panel::addUI(Base *base)
	{
		base->panel = this;
        base->parent = this;
		uis.push_back(base);
        this->addChild(base);
	}
    void Panel::removeUI(Base *base)
    {
        for (BASES_ITER iter = uis.begin(); iter != uis.end();++iter)
        {
            if (base == *iter)
            {
                base->release();
                uis.erase(iter);
                break;
            }
        }
        for (BASES_ITER iter = actives.begin(); iter != actives.end();++iter)
        {
            if (base == *iter)
            {
                base->release();
                actives.erase(iter);
                break;
            }
        }
        for (BASES_ITER iter = firsts.begin(); iter != firsts.end();++iter)
        {
            if (base == *iter)
            {
                base->release();
                firsts.erase(iter);
                break;
            }
        }
    }
	void Panel::visit()
	{
        CCNode::visit();
	}
	/**
	 * 设置坐落的位置
	 */
	void Panel::setLocation(AlignType alignType,const CCSize &splitSize,const CCPoint &gridLocation)
	{
		CCPoint point = getPoint(alignType,splitSize,gridLocation);
		rect.origin = point;
		CCNode::setPosition(point);
	}
	/**
	 * 设置大小
	 */
	void Panel::setSize(const CCSize &size)
	{
		rect.size = size;
        Image::setsize(size);
	}
	CCSize Panel::getSize() // 获取大小
	{
		if (!rect.size.width || !rect.size.height) return Window::getWindowSize();
		return rect.size;
	}
	/**
	 * 获取当前布局产生的位置
	 */
	CCPoint Panel::getPoint(AlignType alignType,const CCSize &splitSize,const CCPoint &gridLocation)
	{
		if (!rect.size.width || !rect.size.height) return Window::getPoint(alignType,splitSize,gridLocation);
		CCSize size = rect.size;
		float x = (size.width / splitSize.width) * gridLocation.x;
		float y = (size.height / splitSize.height) * gridLocation.y;
		if (alignType & myui::ALIGN_RIGHT)
		{
			x = x+splitSize.width;
		}
		if (alignType & myui::ALIGN_UP)
		{
			y = y + splitSize.height;
		}
		if (alignType & myui::ALIGN_CENTER)
		{
			x = x+splitSize.width/2;
			y = y + splitSize.height/2;
		}
		return ccp(x,y);
	}
    void Panel::draw()
    {
        if (sprite) return;
        if (doLuaEvent(HANDLE_DRAW, this, "Panel"))return;
        float width = imageSize.width;
        float height = imageSize.height;
        if (!width || !height)
        {
            width = 500;
            height = 500;
        }
        CCPoint src = ccp(-width/2,-height/2);
        CCPoint dest = ccpAdd(src, ccp(width,height));
        ccColor4F color = {0.5,0.5,1,1};
        ccDrawSolidRect(src, dest, color);
    }
    bool Panel::checkIn(const cocos2d::CCPoint &point)
    {
        if (sprite) return Image::checkIn(point);
        float width = imageSize.width;
        float height = imageSize.height;
        if (!width || !height)
        {
            width = 500;
            height = 500;
        }
		CCPoint pos = this->convertToNodeSpace(point);
		CCRect rect = CCRectMake(
                                 -width/2,
                                 -height/2,
                                 width,
                                 height);
		if (rect.containsPoint(pos))
		{
			return true;
		}
		return false;
        
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////
    // 提供touch事件 获取实际屏幕大小
    /////////////////////////////////////////////////////////////////////////////////////////////
    
	/**
	 * 窗口
	 **/
	Window* Window::create()
	{
		Window *window = new Window;
		if (window && window->init())
		{
			window->autorelease();
		}
		return window;
	}
	bool Window::init()
	{
		this->setTouchEnabled(true);
		return true;
	}
	void Window::ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent)
	{
		CCSetIterator it;
        
		for( it = pTouches->begin(); it != pTouches->end(); ++it)
		{
			CCTouch* touch = (CCTouch*)(*it);
            
			if (firsts.size()) // 优先级的列表
			{
				firsts.back()->attachTouch(TOUCH_DOWN,touch);
				return;
			}
			// 设定actives
			bool tag = false;
            
			for (UIS_ITER iter = uis.begin();iter != uis.end();++iter)
			{
				Base *base = *iter;
				if (base && base->attachTouch(TOUCH_DOWN,touch))
				{
                    if (!tag) actives.clear();
					actives.push_back(base);
					tag  = true;
                    //break;
				}
			}
		}
		return;
	}
	void Window::ccTouchesMoved(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent)
	{
		CCSetIterator it;
        
		for( it = pTouches->begin(); it != pTouches->end(); ++it)
		{
			CCTouch* touch = (CCTouch*)(*it);
			if (firsts.size()) // 优先级的列表
			{
				firsts.back()->attachTouch(TOUCH_DOWN,touch);
				return;
			}
			bool tag = false;
			for (UIS_ITER iter = actives.begin();iter != actives.end();++iter)
			{
				Base *base = *iter;
				if (base && base->attachTouch(TOUCH_MOVE,touch))
					tag = true;
			}
		}
		return;
	}
	void Window::ccTouchesEnded(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent)
	{
		CCSetIterator it;
        
		for( it = pTouches->begin(); it != pTouches->end(); ++it)
		{
			CCTouch* touch = (CCTouch*)(*it);
			if (firsts.size()) // 优先级的列表
			{
				firsts.back()->attachTouch(TOUCH_DOWN,touch);
				return;
			}
			bool tag = false;
            std::list<Base *> temp = uis;
			for (UIS_ITER iter = temp.begin();iter != temp.end();++iter)
			{
				Base *base = *iter;
				if (base && base->attachTouch(TOUCH_END,touch))
					tag = true;
			}
		}
		return;
	}
	void Window::addUI(Base *base)
	{
        base->parent = this;
		uis.push_back(base);
        base->retain();
    }
    
    void Window::removeUI(Base *base)
    {
        for (UIS_ITER iter = uis.begin(); iter != uis.end();++iter)
        {
            if (base == *iter)
            {
                //base->release();
                uis.erase(iter);
                break;
            }
        }
        for (UIS_ITER iter = actives.begin(); iter != actives.end();++iter)
        {
            if (base == *iter)
            {
               // base->release();
                actives.erase(iter);
                break;
            }
        }
        for (UIS_ITER iter = firsts.begin(); iter != firsts.end();++iter)
        {
            if (base == *iter)
            {
                //base->release();
                firsts.erase(iter);
                break;
            }
        }
    }
	void Window::visit()
	{
		// 渲染firsts
		for (UIS_ITER iter = firsts.begin(); iter != firsts.end();++iter)
		{
			Base *base = *iter;
			if (base) base->visit();
		}
		// 渲染actives
		for (UIS_ITER iter = actives.begin(); iter != actives.end();++iter)
		{
			Base *base = *iter;
			if (base)
			{
				//base->isActive = true;
			}
		}
		// 当base 是active 的话不渲染
		for (UIS_ITER iter = uis.begin(); iter != uis.end();++iter)
		{
			Base *base = *iter;
			if (base && !base->isActive)
			{
				base->visit();
			}
			else
			{
				base->isActive = false;
			}
		}
		for (UIS_ITER iter = actives.begin(); iter != actives.end();++iter)
		{
			Base *base = *iter;
			if (base)
			{
				//base->visit();
			}
		}
        CCLayer::visit();
	}
	CCSize Window::getWindowSize()
	{
		return CCDirector::sharedDirector()->getWinSize();
	}
	/**
	 * 获取点的信息
	 * \param clignType 对齐方式
	 * \spliteSize 划分格局
	 * \location 格子编号
	 */
	CCPoint Window::getPoint(AlignType alignType,const CCSize &splitSize,const CCPoint &gridLocation)
	{
		CCSize size = getWindowSize();
		float x = (size.width / splitSize.width) * gridLocation.x;
		float y = (size.height / splitSize.height) * gridLocation.y;
		if (alignType & myui::ALIGN_CENTER)
		{
			x = x+splitSize.width/2;
			y = y + splitSize.height/2;
		}
		if (alignType & myui::ALIGN_RIGHT)
		{
			x = x+splitSize.width;
		}
		if (alignType & myui::ALIGN_UP)
		{
			y = y + splitSize.height;
		}
		return ccp(x,y);
	}
}