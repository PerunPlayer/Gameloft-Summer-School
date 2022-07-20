/* Copyright (C) 2012 Wildfire Games.
 * This file is part of 0 A.D.
 *
 * 0 A.D. is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * 0 A.D. is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 0 A.D.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "precompiled.h"

#include "ScenarioEditor/ScenarioEditor.h"
#include "Common/Tools.h"
#include "Common/Brushes.h"
#include "Common/MiscState.h"
#include "GameInterface/Messages.h"

using AtlasMessage::Position;

class NormalElevation : public StateDrivenTool<NormalElevation>
{
	DECLARE_DYNAMIC_CLASS(NormalElevation);

	Position m_Pos;
	Brush m_Brush;

public:
	NormalElevation()
	{
		m_Brush.SetSquare(5);
		SetState(&Waiting);
	}

	void OnEnable()
	{
		m_Brush.MakeActive();
	}

	void OnDisable()
	{
		POST_MESSAGE(BrushPreview, (false, Position()));
	}

	struct sWaiting : public State
	{
		bool OnMouse(NormalElevation* obj, wxMouseEvent& evt)
		{
			if (evt.LeftDown())
			{
				obj->m_Pos = Position(evt.GetPosition());
				POST_MESSAGE(BrushPreview, (true, obj->m_Pos));
				POST_COMMAND(NormalElevation, (obj->m_Pos, 1024));
				return true;
			}
			else if (evt.Moving())
			{
				POST_MESSAGE(BrushPreview, (true, Position(evt.GetPosition())));
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	Waiting;
};

IMPLEMENT_DYNAMIC_CLASS(NormalElevation, StateDrivenTool<NormalElevation>);
