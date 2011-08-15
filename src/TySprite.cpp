#include <TySprite.h>
#include <s3eConfig.h>

TySprite&	TySprite::operator = (TySprite const& pSprite)
{
	if(this != &pSprite)
	{
		if(pSprite.IsBuilt())
		{
			if(m_Built)
				delete m_Image;
			m_Built = m_Pause = m_Stop = false;
			m_Offset = m_CurrentFrame = m_DrawRect = CIwSVec2::g_Zero;			
			m_FrameCounter = 0;

			LoadFromResource(pSprite.GetResourceGroup(),pSprite.GetResourceName(),false);
			
			m_MaxFrames = pSprite.GetMaxFrames();
			m_Offset = pSprite.GetFrameOffset();
			m_FrameSize = pSprite.GetFrameSize();
			m_Loop = pSprite.IsLooping();
		}		
	}
	return (*this);
}

TySprite::TySprite(CIwResGroup* pGroup, std::string pName, bool pLoop)
	: TyImage(), m_Offset(CIwSVec2::g_Zero), m_CurrentFrame(CIwSVec2::g_Zero), m_FrameCounter(0), m_DrawRect(CIwSVec2::g_Zero),m_Pause(false),m_Stop(false),m_Loop(pLoop)
{
	LoadFromResource(pGroup,pName);
}

TySprite::~TySprite()
{
	if(m_Built)
		delete m_Image;
	m_Built = false;
}

TySprite::TySprite(TySprite const& pSprite)
: TyImage(), m_Offset(CIwSVec2::g_Zero), m_CurrentFrame(CIwSVec2::g_Zero), m_FrameCounter(0), m_DrawRect(CIwSVec2::g_Zero),m_Pause(false),m_Stop(false),m_MaxFrames(pSprite.GetMaxFrames()), m_FrameSize(pSprite.GetFrameSize()),m_Loop(pSprite.IsLooping())
{
	if(pSprite.IsBuilt())
	{
		if(m_Built)
			delete m_Image;
		LoadFromResource(pSprite.GetResourceGroup(),pSprite.GetResourceName(),false);		
	}			
}

bool TySprite::LoadFromResource(CIwResGroup* pGroup, std::string pName, bool pDataFromFile)
{
	bool sameName = ( pName.compare(m_ResourceName) > 0 ) ? true : false;
	bool emptyName = m_ResourceName.empty();
	if( !sameName || m_ResourceGroup != pGroup || !m_Built || ( !emptyName && !m_ResourceGroup) )
	{
		m_ResourceGroup = pGroup;
		m_ResourceName = pName;
		m_Built = false;

		IwGetResManager()->SetCurrentGroup(m_ResourceGroup);
		
		if( m_ResourceGroup->GetResNamed(m_ResourceName.c_str(),IW_GX_RESTYPE_TEXTURE) )
		{				
			m_Built = true;

			if(pDataFromFile)
			{
				std::string w,h,f;
				w = m_ResourceName + "_w" ;
				h = m_ResourceName + "_h" ;
				f = m_ResourceName + "_frames" ;
	
				int temp;
				s3eConfigGetInt("Sprites",w.c_str(),&temp);
				m_FrameSize.x = temp;
				s3eConfigGetInt("Sprites",h.c_str(),&temp);
				m_FrameSize.y = temp;
				s3eConfigGetInt("Sprites",f.c_str(),&temp);	
				m_MaxFrames = temp;
			}

			m_Image = Iw2DCreateImageResource(m_ResourceName.c_str()); 

			m_Frames.x = m_Image->GetWidth() / m_FrameSize.x;
			m_Frames.y = m_Image->GetHeight() / m_FrameSize.y;	
			m_Center.x = m_FrameSize.x / 2;
			m_Center.y = m_FrameSize.y / 2;
			m_Angle = 0;
			m_Flip = CIwSVec2::g_Zero;
		}		
	}
	return m_Built;
}

void TySprite::Render(CIwSVec2 pPosition)
{
	if(m_Angle > 0 || m_Angle < 0)
	{
		CIwMat2D RotationMatrix;
		RotationMatrix.SetRot(m_Angle, CIwVec2(pPosition.x + m_Center.x , pPosition.y + m_Center.y) );
		Iw2DSetTransformMatrix(RotationMatrix);
	}

	if( m_Flip.x > 0 && m_Flip.y > 0)
		Iw2DSetImageTransform(IW_2D_IMAGE_TRANSFORM_ROT180);
	else if( m_Flip.x > 0 && m_Flip.y <= 0)
		Iw2DSetImageTransform(IW_2D_IMAGE_TRANSFORM_FLIP_X);	
	else if( m_Flip.y > 0 && m_Flip.x <= 0)
		Iw2DSetImageTransform(IW_2D_IMAGE_TRANSFORM_FLIP_Y);

	Iw2DDrawImageRegion(m_Image,
						pPosition,
						m_FrameSize,
						m_DrawRect,
						m_FrameSize);

	if(m_Angle > 0 || m_Angle < 0)
		Iw2DSetTransformMatrix(CIwMat2D::g_Identity);
	if( m_Flip.x > 0 || m_Flip.y > 0)
		Iw2DSetImageTransform(IW_2D_IMAGE_TRANSFORM_NONE);
}

bool TySprite::Step()
{
	if(!m_Pause && !m_Stop)
	{	
		++m_CurrentFrame.x;
		if( m_CurrentFrame.x >= m_Frames.x )
		{
			m_CurrentFrame.x = 0;
			++m_CurrentFrame.y;

			if ( m_CurrentFrame.y >= m_Frames.y )
				m_CurrentFrame.y = 0;					
		}	
		
		++m_FrameCounter;
		if( m_FrameCounter >= m_MaxFrames )
		{
			m_Stop = (m_Loop) ? false : true;
			m_CurrentFrame = CIwSVec2::g_Zero;
			m_FrameCounter = 0;
		}

		m_DrawRect.x = m_FrameSize.x * m_CurrentFrame.x;
		m_DrawRect.y = m_FrameSize.y * m_CurrentFrame.y;

	}

	return !m_Stop;
}

void TySprite::Play(){
	m_Pause = m_Stop = false;
}

void TySprite::Resume(){
	if(m_Pause)
		m_Pause = false;
}

void TySprite::Pause(){
		m_Pause = true;
}

void TySprite::Stop(){
	m_Stop = true;
	m_CurrentFrame = CIwSVec2::g_Zero;
}

void TySprite::GoToFrame(CIwSVec2 pFrame){
	if(pFrame.x >= 0 && pFrame.x <= m_Frames.x)
		if(pFrame.y >= 0 && pFrame.y <= m_Frames.y)
			m_CurrentFrame = pFrame;
}