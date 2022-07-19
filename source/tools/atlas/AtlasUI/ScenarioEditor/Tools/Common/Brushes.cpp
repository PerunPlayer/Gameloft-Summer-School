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
	: m_Shape(std::make_unique<BrushCircle>()), m_IsActive(false)
{
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
	m_Shape = std::make_unique<BrushCircle>();
	m_Shape->SetSize(size);
}

void Brush::SetSquare(int size)
{
	m_Shape = std::make_unique<BrushSquare>();
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
		SetSelection(m_Brush.m_Shape->GetID());
	}

private:
	Brush& m_Brush;

	void OnChange(wxCommandEvent& WXUNUSED(evt))
	{
		switch (m_Brush.m_Shape->GetID())
		{
		case 1: m_Brush.m_Shape = std::make_unique<BrushSquare>();
			break;
		case 0: 
		default: m_Brush.m_Shape = std::make_unique<BrushCircle>();
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
	int width = GetDataWidth();
	int height = GetDataHeight();

	std::vector<float> data(width * height);
	int n = 0;
	for (size_t i = 0; i < height; i++)
	{
		for (size_t j = 0; j < width; j++)
		{
			data[n++] = 1.f -(abs((height / 2.0) - i) + abs((width / 2.0) - j)) / (height / sqrt(2));
		}
	}

	return data;
}

std::vector<float> BrushSquare::GetData() const
{
	int width = GetDataWidth();
	int height = GetDataHeight();

	std::vector<float> data(width * height);
	int i = 0;
	for (int y = 0; y < height; ++y)
		for (int x = 0; x < width; ++x)
			data[i++] = 1.f;
	
	return data;
}
