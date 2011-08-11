#include <TyScene.h>

CTyScene::CTyScene(std::string pName, std::string pCaption, std::string pResGroup, int64 pID, bool pZOrdered)
	:	m_Name(pName), m_Caption(pCaption), m_ID(pID), m_ZOrdered(pZOrdered), m_Created(false)
{
	if(! pResGroup.empty())
	{	
		m_ResGroupName = pResGroup;
		m_ResGroup = IwGetResManager()->GetGroupNamed(m_ResGroupName.c_str());
	}
}

CTyScene::~CTyScene()
{
	std::vector<ITyActor*>::iterator it = m_Actors.begin();
	while(it != m_Actors.end())
	{
		delete (*it);
		it = m_Actors.erase(it);
	}
	m_Actors.clear();
}

bool	CTyScene::AddActor(ITyActor* pActor) //adds an actor
{
	bool toadd = false;
	if( pActor )
	{
		toadd = true;
		std::vector<ITyActor*>::iterator it;
		for(it = m_Actors.begin(); it != m_Actors.end(); ++it)
		{
			if( (*it) == pActor )
			{
				toadd = false;
				break;
			}
		}
	}

	if( toadd )
		m_Actors.push_back(pActor);

	return toadd;
}

bool	CTyScene::DeleteActor(ITyActor* pActor) //adds an actor
{
	bool todelete = false;
	if( pActor )
	{		
		std::vector<ITyActor*>::iterator it;
		for(it = m_Actors.begin(); it != m_Actors.end(); ++it)
		{
			if( (*it) == pActor )
			{
				delete (*it);
				m_Actors.erase(it);
				todelete = true;
				break;
			}
		}
	}
	return todelete;
}

bool		CTyScene::Create()
{
	m_Created = false;
	std::vector<ITyActor*>::iterator it;
	for(it = m_Actors.begin(); it != m_Actors.end(); ++it)
	{
		m_Created = (*it)->OnCreate();
		//if( !m_Created ) break; //add exception to show which actor failed to create!
	}
	return m_Created;
}

bool		CTyScene::Destroy()
{
	bool Destroyed = false;
	std::vector<ITyActor*>::iterator it;
	for(it = m_Actors.begin(); it != m_Actors.end(); ++it)
	{
		Destroyed = (*it)->OnDestroy();		
	}	
	if( Destroyed ) m_Created = false;
	return Destroyed;
}

bool		CTyScene::Play()
{
	bool playing = false;
	if( m_Created && m_Actors.size()>0 )
	{
		playing = true;

		std::vector<ITyActor*>::iterator it;

		for( it = m_Actors.begin(); it != m_Actors.end(); ++it )
		{
			if( (*it)->IsActive() )
				(*it)->OnStepStart();
		}

/*		for( it = m_Actors.begin(); it != m_Actors.end(); ++it )
		{
			if( (*it)->IsActive() )
			{
				(*it)->OnKeyPress();
				(*it)->OnKeySustain();
				(*it)->OnKeyRelease();
				(*it)->OnTouchPress();
				(*it)->OnTouchSustain();
				(*it)->OnTouchDrag();
				(*it)->OnTouchRelease();
			}
		} */
		
		for( it = m_Actors.begin(); it != m_Actors.end(); ++it )
		{
			if( (*it)->IsActive() )
				(*it)->OnStep();
		}

		for( it = m_Actors.begin(); it != m_Actors.end(); ++it )
		{
/*			if( (*it)->IsActive() )
				(*it)->OnIntersectBoundaries();
*/
			if( (*it)->IsSolid() )
				for( std::vector<ITyActor*>::iterator it2 = m_Actors.begin(); it2 != m_Actors.end(); ++it2 )
					(*it)->OnCollision( *(*it2) );
		}

		for( it = m_Actors.begin(); it != m_Actors.end(); ++it )
		{
			if( (*it)->IsVisible() )
				(*it)->OnRender();
			if( (*it)->IsActive() )
				(*it)->OnStepEnd();
		}
	}
	return playing;
}
