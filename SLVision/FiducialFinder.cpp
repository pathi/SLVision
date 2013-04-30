/*
	Daniel Gallardo Grassot
	daniel.gallardo@upf.edu
	Barcelona 2011

	Licensed to the Apache Software Foundation (ASF) under one
	or more contributor license agreements.  See the NOTICE file
	distributed with this work for additional information
	regarding copyright ownership.  The ASF licenses this file
	to you under the Apache License, Version 2.0 (the
	"License"); you may not use this file except in compliance
	with the License.  You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing,
	software distributed under the License is distributed on an
	"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
	KIND, either express or implied.  See the License for the
	specific language governing permissions and limitations
	under the License.
*/

#include "FiducialFinder.h"
#include "Globals.h"
#include "GlobalConfig.h"
#include <sstream>
#include <iostream>

FiducialFinder::FiducialFinder(int _fiducial_window_size)
{
	fiducial_window_size = _fiducial_window_size;
//	fiducial_storage = cvCreateMemStorage (0);
//	fiducial_processed_image = cvCreateImage(cvSize(fiducial_window_size,fiducial_window_size),IPL_DEPTH_8U,1);
//	fiducial_blob_moments = (CvMoments*)malloc( sizeof(CvMoments) );
	InitFID();
}

FiducialFinder::~FiducialFinder()
{
//	free(fiducial_blob_moments);
//	cvReleaseImage(&fiducial_processed_image);
//	cvReleaseMemStorage(&fiducial_storage);
}

vector_nodes FiducialFinder::GetLevel (int level, int id, std::vector<cv::Vec4i>& hierarchy)
{
	vector_nodes input;
	input.push_back(node(level,id));
	if(hierarchy[id][2] != -1)
	{
		vector_nodes temp = GetLevel (level+1, hierarchy[id][2], hierarchy);
		for(int i = 0; i < temp.size(); i++)
			input.push_back(temp[i]);
	}
	if(hierarchy[id][0] != -1)
	{
		vector_nodes temp = GetLevel (level, hierarchy[id][0], hierarchy);
		for(int i = 0; i < temp.size(); i++)
			input.push_back(temp[i]);
	}
	return input;
}

int FiducialFinder::DecodeFiducial(cv::Mat& src, Fiducial & candidate)
{
	float x, y;
	float bx, by;
	int counter,axis;


	cv::Mat fid;
	src.copyTo ( fid );
	cv::findContours(fid,contours,hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE );
	//std::cout << "fi" << std::endl;
	fiducial_nodes.clear();

	bx = 0; by =0;
	counter =0;
	if(hierarchy.size()!= 0)
	{
		for(int i = 0; i < hierarchy.size(); i++)
		{
			std::cout << i << "  h0: " << hierarchy[i][0]<< "  h1: " << hierarchy[i][1]<< "  h2: " << hierarchy[i][2]<< "  h3: " << hierarchy[i][3]<< std::endl;
		}
		//std::cout << std::endl;
		//std::cout << "getlevel" << std::endl;
		vector_nodes nodes = GetLevel(0,0,hierarchy);
		for(int i = 0; i< nodes.size(); i++)
		{
			if(nodes[i].first > 1)
			{
				int node_level = abs(nodes[i].first-3);
				//std::cout << node_level << "oo" << nodes[i].second << "  " ;
				fiducial_nodes.push_back(node_level);
				if(node_level == 1)
				{
					cv::Moments fiducial_blob_moments = cv::moments(contours[nodes[i].second],true);
					x = (float)(fiducial_blob_moments.m10 / fiducial_blob_moments.m00);
					y = (float)(fiducial_blob_moments.m01 / fiducial_blob_moments.m00);
					counter++;
					bx += x;
					by += y;
				}
			}
		}

		bx = bx / counter;
		by = by / counter;
		axis = 0;
		//Get the fiducial orientation
		GetMinDistToFiducialAxis(axis,bx,by);
		std::cout <<axis << " " << bx << " " << by<< std::endl;
		//fiducial_nodes.sort();
		int tmpid = GetId(BinaryListToInt(fiducial_nodes));
		std::cout <<tmpid<< std::endl;
		std::cout <<std::endl;
	}
	/*
	for ( unsigned int i=0;i<contours.size();i++ )
	{
		cv::Moments fiducial_blob_moments = cv::moments(contours[i],true);
		x = (float)(fiducial_blob_moments.m10 / fiducial_blob_moments.m00);
		y = (float)(fiducial_blob_moments.m01 / fiducial_blob_moments.m00);

		if(	x > MIN_DIST_TO_SIDE && 
			y > MIN_DIST_TO_SIDE && 
			x <70-MIN_DIST_TO_SIDE && 
			y<70-MIN_DIST_TO_SIDE)
		{
			//if(contours[i
		}
	}*/
/*	float x, y;
	float bx, by;
	int counter;
	int axis;

	if(src->nChannels!=1 || src->width!=src->height)
		return -1;

	CvSeq *contour ;

	cvCopy(src,fiducial_processed_image);
//	Globals::fiducial_image_marker = src;

	cvFindContours (fiducial_processed_image, fiducial_storage, &contour, sizeof (CvContour), CV_RETR_CCOMP);
	fiducial_nodes.clear();
	bx = 0; by =0;
	counter =0;
	for(CvSeq* c=contour;c!=NULL;c=c->h_next)
	{
		//fiducial_blob_moments
		cvMoments( c, fiducial_blob_moments );
		x = (float)(fiducial_blob_moments->m10 / fiducial_blob_moments->m00);
		y = (float)(fiducial_blob_moments->m01 / fiducial_blob_moments->m00);
		if(	x > MIN_DIST_TO_SIDE && 
			y > MIN_DIST_TO_SIDE && 
			x <70-MIN_DIST_TO_SIDE && 
			y<70-MIN_DIST_TO_SIDE)
		{
			//if(show_fid_processor)cvDrawContours(screen,c,CV_RGB(255,255,0),CV_RGB(200,255,255),0);	
			CvSeq * temp;
			temp = c;
			nodecount = 0;
			if(temp->v_next != NULL)
			{
				temp = temp->v_next;
				for(CvSeq* h=temp;h!=NULL;h=h->h_next)
				{
					//if(show_fid_processor)cvDrawContours(screen,h,CV_RGB(255,0,0),CV_RGB(200,0,255),0);
					nodecount ++;
				}
			}
			else
			{
				counter ++;
				bx+=x;
				by+=y;
			}
			fiducial_nodes.push_back(nodecount);
		}
	}
	bx /= counter;
	by /= counter;
	axis = 0;
	//Get the fiducial orientation
	GetMinDistToFiducialAxis(axis,bx,by);
	if(counter != 0)
	candidate.SetOrientation(axis);
	//Get the fiducial ID
	fiducial_nodes.sort();	
	int tmpid = GetId(BinaryListToInt(fiducial_nodes));
	if(tmpid != -1) candidate.SetId(tmpid);*/

	return 1;	
}

float FiducialFinder::GetMinDistToFiducialAxis(int &axis, float x, float y)
{
	float u,l,r,d;
	l = fabs(vect_point_dist(				 0,					0,				   0, FIDUCIAL_WIN_SIZE, x, y));
	r = fabs(vect_point_dist(FIDUCIAL_WIN_SIZE, FIDUCIAL_WIN_SIZE, FIDUCIAL_WIN_SIZE,				  0, x, y));
	u = fabs(vect_point_dist(				 0, FIDUCIAL_WIN_SIZE, FIDUCIAL_WIN_SIZE, FIDUCIAL_WIN_SIZE, x, y));
	d = fabs(vect_point_dist(				 0,					0, FIDUCIAL_WIN_SIZE,				  0, x, y));
	
	if( l < r && l < u && l < d)
	{
		axis = AXIS_LEFT;
		return l;
	}
	else if (r < l && r < u && r < d)
	{
		axis = AXIS_RIGHT;
		return r;
	}
	else if (u < l && u < r && u < d)
	{
		axis = AXIS_UP;
		return u;
	}
	else if (d < l && d < u && d < r)
	{
		axis = AXIS_DOWN;
		return d;
	}
	
	return 0;
}

unsigned int FiducialFinder::BinaryListToInt(const intList &data)
{
	int cnt = 0;
	unsigned int candidate = 0;
	for(intList::const_iterator it = data.begin(); it != data.end(); it++)
	{
		for (int i = 0; i < (*it); i++)
			cnt++;
		candidate += 1<<cnt;
		cnt++;
	}
	return candidate;
}

unsigned int FiducialFinder::StringBinaryListToInt(const char* data)
{
	intList list;
	int i = 0;
	unsigned int candidate = 0;
	while(data[i] != '\0')
	{
		if(data[i] == '1')
			list.push_back(1);
		else list.push_back(0);
		i++;
	}
	i = 0;
	for(intList::reverse_iterator it = list.rbegin(); it != list.rend(); it++)
	{
		if(*it ==1)
		{
			candidate += 1<<i;
		}
		i++;
	}
	return candidate;
}

void FiducialFinder::RepportOSC()
{

}


int FiducialFinder::GetId(unsigned int candidate)
{
	if(idmap.find(candidate) != idmap.end())
	{
		return idmap[candidate];
	}
	return -1;
}

void FiducialFinder::InitFID()
{
	LoadFiducialList();
}

void FiducialFinder::LoadFiducialList ()
{
	idmap.clear();
	
	//<z_fiducialsID>
	//		<count> n <count/>
	//		<fid0> ... <fid0/>
	//		<fidn> ... <fidn/>
	//<z_fiducialsID>

	int &num_fiducials = datasaver::GlobalConfig::getRef("Z_fiducialsID:count",0);
	if(num_fiducials == 0)
	{
		char* id[] = { 
					"10011\0",
					"100111\0",
					"10111\0",
					"1011\0", 
					"111\0",
					"100000101111\0",
					"100010011111\0",
					"100010101111\0",
					"100001011111\0",
					"100100101111\0",
					"100100111111\0",
					"100101011111\0",
					"100001001111\0",
					"101010101111\0",
					"100010001111\0",
					"o"};
		int i = 0;
		while( id[i][0] != 'o' )
		{
			idmap[StringBinaryListToInt(id[i])] = i;
			std::stringstream s;
			s << "Z_fiducialsID:fid_" << i;
			std::string temporal = datasaver::GlobalConfig::getRef<std::string>(s.str().c_str(),std::string(id[i]));
			i++;
		}
		num_fiducials = i;
	}
	else
	{
		for (int i = 0; i < num_fiducials; i++)
		{
			std::stringstream s;
			s << "Z_fiducialsID:fid_" << i;
			std::string temporal = datasaver::GlobalConfig::getRef<std::string>(s.str().c_str(),"0\0");
			idmap[StringBinaryListToInt(temporal.c_str())] = i;
		}
	}
}