/* Copyright (C) 2015 Wildfire Games.
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

#include "Brushes.h"

#include "GameInterface/Messages.h"

#include "wx/spinctrl.h"

Brush g_Brush_Elevation; // shared between several elevation-related tools; other tools have their own brushes

static Brush* g_Brush_CurrentlyActive = NULL; // only one brush can be active at once

const float Brush::STRENGTH_MULTIPLIER = 1024.f;

Brush::Brush()
	: m_IsActive(false)
{
	m_Shape.reset(new BrushCircle);
}

Brush::~Brush()
{
	// Avoid dangling pointers
	if (g_Brush_CurrentlyActive == this)
		g_Brush_CurrentlyActive = NULL;
}

void Brush::MakeActive()
{
	if (g_Brush_CurrentlyActive)
		g_Brush_CurrentlyActive->m_IsActive = false;

	g_Brush_CurrentlyActive = this;
	m_IsActive = true;

	Send();
}

void Brush::Send()
{
	if (m_IsActive)
		POST_MESSAGE(Brush, (m_Shape->GetDataWidth(), m_Shape->GetDataHeight(), m_Shape->GetData()));
}

void Brush::SetCircle(int size)
{
	m_Shape.reset(new BrushCircle);
	m_Shape->SetSize(size);
}

void Brush::SetSquare(int size)
{
	m_Shape.reset(new BrushSquare);
	m_Shape->SetSize(size);
}

void Brush::SetPyramid(int size)
{
	m_Shape.reset(new BrushPyramid);;
	m_Shape->SetSize(size);
}

void Brush::SetRidge(int size)
{
	m_Shape.reset(new BrushRidge);;
	m_Shape->SetSize(size);
}

//////////////////////////////////////////////////////////////////////////

class BrushShapeCtrl : public wxRadioBox
{
public:
	BrushShapeCtrl(wxWindow* parent, wxArrayString& shapes, Brush& brush)
		: wxRadioBox(parent, wxID_ANY, _("Shape"), wxDefaultPosition, wxDefaultSize, shapes, 0, wxRA_SPECIFY_ROWS),
		m_Brush(brush)
	{
		SetSelection(0);
	}

private:
	Brush& m_Brush;

	void OnChange(wxCommandEvent& WXUNUSED(evt))
	{
		int n = m_Brush.m_Shape->GetID();
		switch (GetSelection())
		{
		case 3:m_Brush.SetRidge(16);
			break;
		case 2: m_Brush.SetPyramid(16);
			break;
		case 1: m_Brush.SetSquare(16);
			break;
		case 0:
		default: m_Brush.SetCircle(16);
			break;
		}
		m_Brush.Send();
	}

	DECLARE_EVENT_TABLE();
};
BEGIN_EVENT_TABLE(BrushShapeCtrl, wxRadioBox)
EVT_RADIOBOX(wxID_ANY, BrushShapeCtrl::OnChange)
END_EVENT_TABLE()


class BrushSizeCtrl : public wxSpinCtrl
{
public:
	BrushSizeCtrl(wxWindow* parent, Brush& brush)
		: wxSpinCtrl(parent, wxID_ANY, wxString::Format(_T("%d"), brush.m_Shape->GetSize()),
			wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, brush.m_Shape->GetSize()),
		m_Brush(brush)
	{
	}

private:
	Brush& m_Brush;

	void OnChange(wxSpinEvent& WXUNUSED(evt))
	{
		m_Brush.m_Shape->SetSize(GetValue());
		m_Brush.Send();
	}

	DECLARE_EVENT_TABLE();
};
BEGIN_EVENT_TABLE(BrushSizeCtrl, wxSpinCtrl)
EVT_SPINCTRL(wxID_ANY, BrushSizeCtrl::OnChange)
END_EVENT_TABLE()


class BrushStrengthCtrl : public wxSpinCtrl
{
public:
	BrushStrengthCtrl(wxWindow* parent, Brush& brush)
		: wxSpinCtrl(parent, wxID_ANY, wxString::Format(_T("%d"), (int)(10.f * brush.m_Shape->GetStrength())),
			wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, (int)(10.f * brush.m_Shape->GetStrength())),
		m_Brush(brush)
	{
	}

private:
	Brush& m_Brush;

	void OnChange(wxSpinEvent& WXUNUSED(evt))
	{
		m_Brush.m_Shape->SetStrength(GetValue() / 10.f);
		m_Brush.Send();
	}

	DECLARE_EVENT_TABLE();
};
BEGIN_EVENT_TABLE(BrushStrengthCtrl, wxSpinCtrl)
EVT_SPINCTRL(wxID_ANY, BrushStrengthCtrl::OnChange)
END_EVENT_TABLE()



void Brush::CreateUI(wxWindow* parent, wxSizer* sizer)
{
	wxArrayString shapes; // Must match order of BrushShape enum
	shapes.Add(_("Circle"));
	shapes.Add(_("Square"));
	shapes.Add(_("Pyramid"));
	shapes.Add(_("Ridge"));
	// TODO (maybe): get rid of the extra static box, by not using wxRadioBox
	sizer->Add(new BrushShapeCtrl(parent, shapes, *this), wxSizerFlags().Expand());

	sizer->AddSpacer(5);

	// TODO: These are yucky
	wxFlexGridSizer* spinnerSizer = new wxFlexGridSizer(2, 5, 5);
	spinnerSizer->AddGrowableCol(1);
	spinnerSizer->Add(new wxStaticText(parent, wxID_ANY, _("Size")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	spinnerSizer->Add(new BrushSizeCtrl(parent, *this), wxSizerFlags().Expand());
	spinnerSizer->Add(new wxStaticText(parent, wxID_ANY, _("Strength")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	spinnerSizer->Add(new BrushStrengthCtrl(parent, *this), wxSizerFlags().Expand());
	sizer->Add(spinnerSizer, wxSizerFlags().Expand());
}

std::vector<float> BrushCircle::GetData() const
{
	int height = GetDataHeight();

	std::vector<float> data(height * height);
	int n = 0;
	// All calculations are done in units of half-tiles, since that
	// is the required precision
	int mid_x = height - 1;
	int mid_y = height - 1;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < height; j++)
		{
			float dist_sq = // scaled to 0 in centre, 1 on edge
				((2 * i - mid_x) * (2 * i - mid_x) +
					(2 * j - mid_y) * (2 * j - mid_y)) / (float)(height * height);
			if (dist_sq <= 1.f)
				data[n++] = (sqrtf(2.f - dist_sq) - 1.f) / (sqrt(2.f) - 1.f);
			else
				data[n++] = 0.f;
		}
	}

	return data;
}

std::vector<float> BrushSquare::GetData() const
{
	int height = GetDataHeight();

	std::vector<float> data(height * height);
	int i = 0;
	for (int y = 0; y < height; ++y)
		for (int x = 0; x < height; ++x)
			data[i++] = 1.f;

	return data;
}

std::vector<float> BrushPyramid::GetData() const
{
	int height = GetDataHeight();

	std::vector<float> data(height * height);
	float half = float(height) / 2;
	int n = 0;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < height; j++)
		{
			float northSouth = 1 - abs(half - j) / half;
			float eastWest = 1 - abs(half - i) / half;
			if (northSouth < eastWest)
				data[n++] = northSouth;
			else
				data[n++] = eastWest;
		}
	}

	return data;
}

std::vector<float> BrushRidge::GetData() const
{
	int height = GetDataHeight();

	std::vector<float> data(height * height);
	float half = float(height) / 2;
	int n = 0;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < height; j++)
		{
			float northSouth = 1 - abs(half - j) / half;
				data[n++] = northSouth;
		}
	}

	return data;
}
