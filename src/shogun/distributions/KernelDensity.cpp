/*
 * Copyright (c) The Shogun Machine Learning Toolbox
 * Written (w) 2014 Parijat Mazumdar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the Shogun Development Team.
 */

#include <shogun/distributions/KernelDensity.h>
#include <shogun/features/DenseFeatures.h>
#include <shogun/multiclass/tree/KDTree.h>
#include <shogun/multiclass/tree/BallTree.h>

using namespace shogun;

KernelDensity::KernelDensity(float64_t bandwidth, EKernelType kernel, EDistanceType dist, EEvaluationMode eval, int32_t leaf_size, float64_t atol, float64_t rtol)
: Distribution()
{
	init();

	m_bandwidth=bandwidth;
	m_eval=eval;
	m_leaf_size=leaf_size;
	m_atol=atol;
	m_rtol=rtol;
	m_dist=dist;
	m_kernel_type=kernel;
}

KernelDensity::~KernelDensity()
{

}

bool KernelDensity::train(std::shared_ptr<Features> data)
{
	require(data,"Data not supplied");
	auto dense_data=std::dynamic_pointer_cast<DenseFeatures<float64_t>>(data);

	switch (m_eval)
	{
		case EM_KDTREE_SINGLE:
		{
			tree=std::make_shared<KDTree>(m_leaf_size,m_dist);
			break;
		}
		case EM_BALLTREE_SINGLE:
		{
			tree=std::make_shared<BallTree>(m_leaf_size,m_dist);
			break;
		}
		case EM_KDTREE_DUAL:
		{
			tree=std::make_shared<KDTree>(m_leaf_size,m_dist);
			break;
		}
		case EM_BALLTREE_DUAL:
		{
			tree=std::make_shared<BallTree>(m_leaf_size,m_dist);
			break;
		}
		default:
		{
			error("Evaluation mode not recognized");
		}
	}

	tree->build_tree(dense_data);
	return true;
}

SGVector<float64_t> KernelDensity::get_log_density(const std::shared_ptr<Features>& test, int32_t leaf_size)
{
	require(test,"data not supplied");
	auto dense_feat =std::dynamic_pointer_cast<DenseFeatures<float64_t>>(test);
	require(dense_feat,"Expected DenseFeatures<float64_t> type");

	if ((m_eval==EM_KDTREE_SINGLE) || (m_eval==EM_BALLTREE_SINGLE))
		return tree->log_kernel_density(dense_feat->get_feature_matrix(),m_kernel_type,m_bandwidth,m_atol,m_rtol);

	std::shared_ptr<CNbodyTree> query_tree=NULL;
	if (m_eval==EM_KDTREE_DUAL)
		query_tree=std::make_shared<KDTree>(leaf_size,m_dist);
	else if (m_eval==EM_BALLTREE_DUAL)
		query_tree=std::make_shared<BallTree>(leaf_size,m_dist);
	else
		error("Evaluation mode not identified");

	query_tree->build_tree(dense_feat);
	std::shared_ptr<BinaryTreeMachineNode<NbodyTreeNodeData>> qroot=NULL;
	auto root=query_tree->get_root();
	if (root)
		qroot=std::dynamic_pointer_cast<BinaryTreeMachineNode<NbodyTreeNodeData>>(root);
	else
		error("Query tree root not found!");

	SGVector<index_t> qid=query_tree->get_rearranged_vector_ids();
	SGVector<float64_t> ret=tree->log_kernel_density_dual(dense_feat->get_feature_matrix(),qid,qroot,m_kernel_type,m_bandwidth,m_atol,m_rtol);




	return ret;
}

int32_t KernelDensity::get_num_model_parameters()
{
	not_implemented(SOURCE_LOCATION);;
	return 0;
}

float64_t KernelDensity::get_log_model_parameter(int32_t num_param)
{
	not_implemented(SOURCE_LOCATION);;
	return 0;
}

float64_t KernelDensity::get_log_derivative(int32_t num_param, int32_t num_example)
{
	not_implemented(SOURCE_LOCATION);;
	return 0;
}

float64_t KernelDensity::get_log_likelihood_example(int32_t num_example)
{
	not_implemented(SOURCE_LOCATION);;
	return 0;
}

void KernelDensity::init()
{
	m_bandwidth=1.0;
	m_eval=EM_KDTREE_SINGLE;
	m_kernel_type=K_GAUSSIAN;
	m_dist=D_EUCLIDEAN;
	m_leaf_size=1;
	m_atol=0;
	m_rtol=0;
	tree=NULL;

	SG_ADD(&m_bandwidth,"m_bandwidth","bandwidth");
	SG_ADD(&m_leaf_size,"m_leaf_size","leaf size");
	SG_ADD(&m_atol,"m_atol","absolute tolerance");
	SG_ADD(&m_rtol,"m_rtol","relative tolerance");
	SG_ADD((std::shared_ptr<SGObject>*) &tree,"tree","tree");
}
