// Copyright (c) 2015-2018, CNRS
// Authors: Justin Carpentier <jcarpent@laas.fr>

#ifndef __multicontact_api_serialization_aligned_vector_hpp__
#define __multicontact_api_serialization_aligned_vector_hpp__

#include <pinocchio/container/aligned-vector.hpp>

#include <boost/serialization/split_free.hpp>
#include <boost/serialization/vector.hpp>

namespace boost{

  namespace serialization{

    template <class Archive, typename T>
    void save(Archive & ar, const pinocchio::container::aligned_vector<T> & v, const unsigned int version)
    {
      typedef typename pinocchio::container::aligned_vector<T>::vector_base vector_base;
      save(ar, *static_cast<const vector_base*>(&v), version);
    }

    template <class Archive, typename T>
    void load(Archive & ar, pinocchio::container::aligned_vector<T> & v, const unsigned int version)
    {
      typedef typename pinocchio::container::aligned_vector<T>::vector_base vector_base;
      load(ar, *static_cast<vector_base*>(&v), version);
    }

    template <class Archive, typename T>
    void serialize(Archive & ar, pinocchio::container::aligned_vector<T> & v, const unsigned int version)
    {
      split_free(ar,v,version);
    }

  }

}

#endif // ifndef __multicontact_api_serialization_aligned_vector_hpp__
