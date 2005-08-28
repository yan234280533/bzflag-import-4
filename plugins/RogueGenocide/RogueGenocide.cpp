// rogueGenocide.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <string>
#include <map>

BZ_GET_PLUGIN_VERSION

// event handler callback
class RogueDeathHandler : public bz_EventHandler
{
public:
        RogueDeathHandler();
        virtual ~RogueDeathHandler();

        virtual void process ( bz_EventData *eventData );

        virtual bool autoDelete ( void ) { return false;} // this will be used for more then one event
protected:
};

RogueDeathHandler	deathHandler;

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
	bz_debugMessage(4,"rogueGenocide plugin loaded");
	bz_registerEvent(bz_ePlayerDieEvent,&deathHandler);

	return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_removeEvent(bz_ePlayerDieEvent,&deathHandler);
	bz_debugMessage(4,"rogueGenocide plugin unloaded");
	return 0;
}


RogueDeathHandler::RogueDeathHandler()
{
}

RogueDeathHandler::~RogueDeathHandler()
{
}

void RogueDeathHandler::process ( bz_EventData *eventData )
{	
	switch (eventData->eventType) 
	{
		default:
			// no clue
			break;
			
		// wait for a tank death and start checking for genocide and rogues
		case bz_ePlayerDieEvent: 
		{
			bz_PlayerDieEventData	*dieData = (bz_PlayerDieEventData*)eventData;
			//if its not a genocide kill, dont care
			if (dieData->flagKilledWith != "G" )
				break;
			// if the tank killed was not a rogue, let the server/client do the normal killing
			if (dieData->team != eRogueTeam )
				break;
			// if the tank killed was a rogue, and the killer was a rogue, kill the lousy tk'er
			if (dieData->killerTeam == eRogueTeam ) 
			{
			//	bz_killPlayer ( dieData->killerID, 0, dieData->killerID, "G" );
				bz_sendTextMessage(BZ_SERVER,dieData->killerID,"You should be more careful with Genocide");
			}
			// if the tank killed was a rogue, and the killer wasnt, kill all rogues.
			// note the possible issue if the rogue tank being killed has not spawned
			// and if all else fails, just kill all rogues.....nothing wrong with that

			bzAPIIntList	*playerList = bz_newIntList();

			bz_getPlayerIndexList(playerList);

			for ( unsigned int i = 0; i < playerList->size(); i++)
			{
				int targetID = (*playerList)[i];
				bz_PlayerRecord *playRec = bz_getPlayerByIndex ( targetID );
				if (!playRec) continue;

				if ( playRec->spawned && playRec->team == eRogueTeam )
				{
					bz_killPlayer( targetID, false, dieData->killerID, "G" );
					bz_sendTextMessage(BZ_SERVER, targetID, "You were a victim of Rogue Genocide");
				}
				bz_freePlayerRecord(playRec);
			}
			bz_deleteIntList(playerList);
		}
		break;
		
	}
}
