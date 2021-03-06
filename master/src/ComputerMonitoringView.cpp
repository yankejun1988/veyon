/*
 * ComputerMonitoringView.cpp - provides a view with computer monitor thumbnails
 *
 * Copyright (c) 2017-2020 Tobias Junghans <tobydox@veyon.io>
 *
 * This file is part of Veyon - https://veyon.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "ComputerControlListModel.h"
#include "ComputerManager.h"
#include "ComputerMonitoringView.h"
#include "ComputerMonitoringModel.h"
#include "VeyonMaster.h"
#include "FeatureManager.h"
#include "VeyonConfiguration.h"
#include "UserConfig.h"


ComputerMonitoringView::ComputerMonitoringView() :
	m_master( VeyonCore::instance()->findChild<VeyonMaster *>() )
{
}



void ComputerMonitoringView::initializeView()
{
	setColors( VeyonCore::config().computerMonitoringBackgroundColor(),
			   VeyonCore::config().computerMonitoringTextColor() );

	setComputerScreenSize( m_master->userConfig().monitoringScreenSize() );

	loadComputerPositions( m_master->userConfig().computerPositions() );
	setUseCustomComputerPositions( m_master->userConfig().useCustomComputerPositions() );
}



void ComputerMonitoringView::saveConfiguration()
{
	m_master->userConfig().setFilterPoweredOnComputers( listModel()->stateFilter() != ComputerControlInterface::State::None );
	m_master->userConfig().setComputerPositions( saveComputerPositions() );
	m_master->userConfig().setUseCustomComputerPositions( useCustomComputerPositions() );
}



QString ComputerMonitoringView::searchFilter() const
{
	return listModel()->filterRegExp().pattern();
}



void ComputerMonitoringView::setSearchFilter( const QString& searchFilter )
{
	listModel()->setFilterRegExp( searchFilter );
}



void ComputerMonitoringView::setFilterPoweredOnComputers( bool enabled )
{
	listModel()->setStateFilter( enabled ? ComputerControlInterface::State::Connected :
										   ComputerControlInterface::State::None );
}



QStringList ComputerMonitoringView::groupFilter() const
{
	return listModel()->groupsFilter().toList();
}



void ComputerMonitoringView::setGroupFilter( const QStringList& groups )
{
	listModel()->setGroupsFilter( groups );
}



void ComputerMonitoringView::setComputerScreenSize( int size )
{
	if( m_computerScreenSize != size )
	{
		const auto minSize = MinimumComputerScreenSize;
		const auto maxSize = MaximumComputerScreenSize;
		size = qBound<int>( minSize, size, maxSize );

		m_computerScreenSize = size;

		m_master->userConfig().setMonitoringScreenSize( size );

		m_master->computerControlListModel().updateComputerScreenSize();

		setIconSize( QSize( size, size * 9 / 16 ) );
	}
}



int ComputerMonitoringView::computerScreenSize() const
{
	return m_computerScreenSize;
}



void ComputerMonitoringView::runFeature( const Feature& feature )
{
	auto computerControlInterfaces = selectedComputerControlInterfaces();
	if( computerControlInterfaces.isEmpty() )
	{
		computerControlInterfaces = m_master->filteredComputerControlInterfaces();
	}

	// mode feature already active?
	if( feature.testFlag( Feature::Mode ) &&
		activeFeatures( computerControlInterfaces ).contains( feature.uid().toString() ) )
	{
		// then stop it
		m_master->featureManager().stopFeature( *m_master, feature, computerControlInterfaces );
	}
	else
	{
		// stop all other active mode feature
		if( feature.testFlag( Feature::Mode ) )
		{
			for( const auto& currentFeature : m_master->features() )
			{
				if( currentFeature.testFlag( Feature::Mode ) && currentFeature != feature )
				{
					m_master->featureManager().stopFeature( *m_master, currentFeature, computerControlInterfaces );
				}
			}
		}

		m_master->featureManager().startFeature( *m_master, feature, computerControlInterfaces );
	}
}



ComputerMonitoringModel* ComputerMonitoringView::listModel() const
{
	return m_master->computerMonitoringModel();
}



FeatureUidList ComputerMonitoringView::activeFeatures( const ComputerControlInterfaceList& computerControlInterfaces )
{
	FeatureUidList featureUidList;

	for( const auto& controlInterface : computerControlInterfaces )
	{
		featureUidList.append( controlInterface->activeFeatures() );
	}

	featureUidList.removeDuplicates();

	return featureUidList;
}
