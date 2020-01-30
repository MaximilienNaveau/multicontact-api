#ifndef __multicontact_api_scenario_contact_sequence_hpp__
#define __multicontact_api_scenario_contact_sequence_hpp__

#include "multicontact-api/scenario/fwd.hpp"
#include "multicontact-api/scenario/contact-phase.hpp"

#include "multicontact-api/serialization/archive.hpp"

#include <vector>
#include <boost/serialization/vector.hpp>

namespace multicontact_api {
namespace scenario {

template <class _ContactPhase>
struct ContactSequenceTpl : public serialization::Serializable<ContactSequenceTpl<_ContactPhase> > {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  typedef _ContactPhase ContactPhase;
  typedef typename ContactPhase::Scalar Scalar;
  typedef typename ContactPhase::t_strings t_strings;
  typedef std::vector<ContactPhase> ContactPhaseVector;

  ContactSequenceTpl(const size_t size = 0) : m_contact_phases(size) {}

  /// \brief Copy contructor
  ContactSequenceTpl(const ContactSequenceTpl& other) : m_contact_phases(other.m_contact_phases) {}

  size_t size() const { return m_contact_phases.size(); }

  bool operator==(const ContactSequenceTpl& other) const { return m_contact_phases == other.m_contact_phases; }

  bool operator!=(const ContactSequenceTpl& other) const { return !(*this == other); }

  void resize(const size_t size) { m_contact_phases.resize(size); }

  /* Accessors to the contact Phases */

  /**
   * @brief append Add the given Phase at the end of the sequence
   * @param contactPhase the phase to end
   * @return The id of the phase added in the sequence
   */
  size_t append(const ContactPhase& contactPhase) {
    m_contact_phases.push_back(contactPhase);
    return m_contact_phases.size() - 1;
  }

  /**
   * @brief contactPhases return a Const copy of the contact phase vector in this sequence.
   * Prefer accessing the contact phases through the contactPhase(id) as this one create a copy
   * @return a Const copy of the contact phase vector in this sequence
   */
  const ContactPhaseVector contactPhases() const { return m_contact_phases; }

  /**
   * @brief contactPhase return a reference to the contactPhase stored at the given Id
   * @param id the desired Id in the contact sequence
   * @return a reference to the ContactPhase
   */
  ContactPhase& contactPhase(const size_t id) {
    if (id >= m_contact_phases.size())
      // throw std::invalid_argument("Contact Sequence size is "+m_contact_phases.size()+" given Id is "+id);
      throw std::invalid_argument("Given Id is greater than the vector size");
    return m_contact_phases.at(id);
  }

  /**
   * @brief removePhase remove the given contactPhase from the sequence
   * @param id the Id of the phase to remove
   */
  void removePhase(const size_t id) {
    if (id >= m_contact_phases.size()) throw std::invalid_argument("Given Id is greater than the vector size");
    m_contact_phases.erase(m_contact_phases.begin() + id);
  }

  /* End Accessors to the contact Phases */

  /* Helpers */
  /**
   * @brief breakContact Add a new contactPhase at the end of the current ContactSequence,
   * The new ContactPhase have the same ContactPatchs as the last phase of the sequence,
   * with the exeption of the given contact removed.
   * It copy all the 'final' values of the last phase as 'initial' values of the new phase.
   * It also set the duration of the previous last phase.
   * @param eeName the name of the effector to remove from contact
   * @param phaseDuration if provided, the duration of the previous last phase of the sequence is set to this value
   * (it is thus the duration BEFORE breaking the contact)
   * @throw invalid_argument if the phaseDuration is provided but the last phase do not have a time-range defined
   * @throw invalid_argument if eeName is not in contact in the last phase of the sequence
   */
  void breakContact(const std::string& eeName, const double phaseDuration = -1) {
    ContactPhase& lastPhase = m_contact_phases.back();
    if (!lastPhase.isEffectorInContact(eeName))
      throw std::invalid_argument("In breakContact : effector is not currently in contact : " + eeName);
    if (phaseDuration > 0) {
      if (lastPhase.timeInitial() < 0) {
        throw std::invalid_argument(
            "In breakContact : duration is specified but current phase interval in not initialised.");
      } else {
        lastPhase.duration(phaseDuration);
      }
    }
    ContactPhase phase;
    // set initial values from last values of previous phase :
    phase.timeInitial(lastPhase.timeFinal());
    phase.m_c_init = lastPhase.m_c_final;
    phase.m_dc_init = lastPhase.m_dc_final;
    phase.m_ddc_init = lastPhase.m_ddc_final;
    phase.m_L_init = lastPhase.m_L_final;
    phase.m_dL_init = lastPhase.m_dL_final;
    phase.m_q_init = lastPhase.m_q_final;

    // copy contact patchs :
    for (std::string name : lastPhase.effectorsInContact()) {
      if (name != eeName) {
        phase.addContact(name, lastPhase.contactPatch(name));
      }
    }
    // add new phase at the end of the sequence
    append(phase);
    return;
  }

  /**
   * @brief createContact Add a new contactPhase at the end of the current ContactSequence,
   * The new ContactPhase have the same ContactPatchs as the last phase of the sequence,
   * with the exeption of the given contact added.
   * @param eeName the name of the effector used to create contact
   * @param patch the ContactPatch of the new contact
   * @param phaseDuration if provided, the duration of the previous last phase of the sequence is set to this value
   * (it is thus the duration BEFORE creating the contact)
   * @throw invalid_argument if the phaseDuration is provided but the last phase do not have a time-range defined
   * @throw invalid_argument if eeName is already in contact in the last phase of the sequence
   */
  void createContact(const std::string& eeName, const ContactPatch& patch, const double phaseDuration = -1) {
    ContactPhase& lastPhase = m_contact_phases.back();
    if (lastPhase.isEffectorInContact(eeName))
      throw std::invalid_argument("In createContact : effector is already in contact : " + eeName);
    if (phaseDuration > 0) {
      if (lastPhase.timeInitial() < 0) {
        throw std::invalid_argument(
            "In createContact : duration is specified but current phase interval in not initialised.");
      } else {
        lastPhase.duration(phaseDuration);
      }
    }
    ContactPhase phase;
    // set initial values from last values of previous phase :
    phase.timeInitial(lastPhase.timeFinal());
    phase.m_c_init = lastPhase.m_c_final;
    phase.m_dc_init = lastPhase.m_dc_final;
    phase.m_ddc_init = lastPhase.m_ddc_final;
    phase.m_L_init = lastPhase.m_L_final;
    phase.m_dL_init = lastPhase.m_dL_final;
    phase.m_q_init = lastPhase.m_q_final;

    // copy contact patchs :
    for (std::string name : lastPhase.effectorsInContact()) {
      phase.addContact(name, lastPhase.contactPatch(name));
    }
    // add new contact to new phase :
    phase.addContact(eeName, patch);
    // add new phase at the end of the sequence
    append(phase);
    return;
  }

  /**
   * @brief moveEffectorToPlacement Add two new phases at the end of the current ContactSequence,
   *  - it break the contact with eeName
   *  - it create the contact with eeName at the given placement.
   * It copy all the 'final' values of the last phase as 'initial' values of the new phase.
   * It also set the duration of the previous last phase.
   * @param eeName the name of the effector used to create contact
   * @param placement the new placement for the contact of eeName
   * @param durationBreak the duration of the previous last phase of the sequence
   *  (it is thus the duration BEFORE breaking the contact)
   * @param durationCreate the duration of the first new ContactPhase
   *  (it is thus the duration BEFORE creating the contact)
   * @throw invalid_argument if the phaseDuration is provided but the last phase do not have a time-range defined
   * @throw invalid_argument if eeName is not in contact in the last phase of the sequence
   */
  void moveEffectorToPlacement(const std::string& eeName, const ContactPatch::SE3& placement,
                               const double durationBreak = -1, const double durationCreate = -1) {
    if (!m_contact_phases.back().isEffectorInContact(eeName))
      throw std::invalid_argument("In moveEffectorToPlacement : effector is not currently in contact : " + eeName);
    ContactPatch target(m_contact_phases.back().contactPatch(eeName));
    target.placement() = placement;
    breakContact(eeName, durationBreak);
    // copy all "init" value to "final" for the current last phase :
    ContactPhase& lastPhase = m_contact_phases.back();
    lastPhase.m_c_final = lastPhase.m_c_init;
    lastPhase.m_dc_final = lastPhase.m_dc_init;
    lastPhase.m_ddc_final = lastPhase.m_ddc_init;
    lastPhase.m_L_final = lastPhase.m_L_init;
    lastPhase.m_dL_final = lastPhase.m_dL_init;
    lastPhase.m_q_final = lastPhase.m_q_init;
    createContact(eeName, target, durationCreate);
    return;
  }

  /**
   * @brief moveEffectorOf similar to moveEffectorToPlacement
   * exept that the new placement is defined from the previous placement and a given transform applied.
   * @param eeName the name of the effector used to create contact
   * @param transform the transform applied to the placement of the contact in the last phase of the sequence
   * @param durationBreak the duration of the previous last phase of the sequence
   *  (it is thus the duration BEFORE breaking the contact)
   * @param durationCreate the duration of the first new ContactPhase
   *  (it is thus the duration BEFORE creating the contact)
   * @throw invalid_argument if the phaseDuration is provided but the last phase do not have a time-range defined
   * @throw invalid_argument if eeName is not in contact in the last phase of the sequence
   */
  void moveEffectorOf(const std::string& eeName, const ContactPatch::SE3& transform, const double durationBreak = -1,
                      const double durationCreate = -1) {
    if (!m_contact_phases.back().isEffectorInContact(eeName))
      throw std::invalid_argument("In moveEffectorToPlacement : effector is not currently in contact : " + eeName);
    ContactPatch::SE3 previous(m_contact_phases.back().contactPatch(eeName).placement());
    ContactPatch::SE3 target = previous.act(transform);
    return moveEffectorToPlacement(eeName, target, durationBreak, durationCreate);
  }

  /**
   * @brief haveTimings Check if all the time intervals are defined and consistent
   * (ie. the time always increase and the final time of one phase is equal to the initial one of the newt phase)
   * @return true if the sequence is consistent, false otherwise
   */
  bool haveTimings() const {
    double current_t = m_contact_phases.front().timeInitial();
    if (current_t < 0.) {
      std::cout << "Initial time is negative." << std::endl;
      return false;
    }
    for (size_t i = 0; i < size(); ++i) {
      const ContactPhase& phase = m_contact_phases.at(i);
      if (phase.timeInitial() != current_t) {
        std::cout << "For phase " << i << " initial time is not equal to previous final time." << std::endl;
        return false;
      }
      if (phase.timeInitial() > phase.timeFinal()) {
        std::cout << "For phase " << i << " final time is before initial time." << std::endl;
        return false;
      }
      current_t = phase.timeFinal();
    }
    return true;
  }

  /**
   * @brief haveConsistentContacts check that there is always one contact change between adjacent phases in the sequence
   * and that there isn't any phase without any contact.
   * @return
   */
  bool haveConsistentContacts() const{
    size_t variations;
    if(m_contact_phases.front().numContacts() == 0){
      // FIXME : may want to test this in a separate method in the future
      std::cout<<"Phase without any contact at id : 0"<<std::endl;
      return false;
    }
    for(size_t i = 1 ; i < m_contact_phases.size() ; ++i){
      variations = m_contact_phases.at(i-1).getContactsBroken(m_contact_phases.at(i)).size()
          + m_contact_phases.at(i-1).getContactsCreated(m_contact_phases.at(i)).size();
      if(variations > 1){
        std::cout<<"Several contact changes between adjacents phases at id : "<<i<<std::endl;
        return false;
      }
      if(m_contact_phases.at(i-1).getContactsRepositioned(m_contact_phases.at(i)).size() > 0){
        std::cout<<"Contact repositionning without intermediate phase at id : "<<i<<std::endl;
        return false;
      }
      if(m_contact_phases.at(i).numContacts() == 0){
        // FIXME : may want to test this in a separate method in the future
        std::cout<<"Phase without any contact at id : "<<i<<std::endl;
        return false;
      }
      if(variations == 0){
        std::cout<<"No contact change between adjacents phases at id : "<<i<<std::endl;
        return false;
      }
    }
    return true;
  }

  /**
   * @brief haveCOMvalues Check that the initial and final CoM position values are defined for all phases
   * Also check that the initial values of one phase correspond to the final values of the previous ones.
   * @return
   */
  bool haveCOMvalues() const{
    if(m_contact_phases.front().m_c_init.isZero()){
      std::cout<<"Initial CoM position not defined."<<std::endl;
      return false;
    }
    for(size_t i = 1 ; i < m_contact_phases.size() ; ++i){
      if(m_contact_phases.at(i).m_c_init.isZero()){
        std::cout<<"Intermediate CoM position not defined for phase : "<<i<<std::endl;
        return false;
      }
      if(m_contact_phases.at(i).m_c_init != m_contact_phases.at(i-1).m_c_final){
        std::cout<<"Init CoM position do not match final CoM of previous phase for id : "<<i<<std::endl;
        return false;
      }
      if(!m_contact_phases.at(i).m_dc_init.isZero()){
        if(m_contact_phases.at(i).m_dc_init != m_contact_phases.at(i-1).m_dc_final){
          std::cout<<"Init CoM velocity do not match final velocity of previous phase for id : "<<i<<std::endl;
          return false;
        }
      }
      if(!m_contact_phases.at(i).m_ddc_init.isZero()){
        if(m_contact_phases.at(i).m_ddc_init != m_contact_phases.at(i-1).m_ddc_final){
          std::cout<<"Init CoM acceleration do not match final acceleration of previous phase for id : "<<i<<std::endl;
          return false;
        }
      }
    }
    if(m_contact_phases.back().m_c_final.isZero()){
      std::cout<<"Final CoM position not defined."<<std::endl;
      return false;
    }
    return true;
  }

  /**
   * @brief haveAMvalues Check that the initial and final AM values are defined for all phases
   * Also check that the initial values of one phase correspond to the final values of the previous ones.
   * @return
   */
  bool haveAMvalues() const{
    for(size_t i = 1 ; i < m_contact_phases.size() ; ++i){
      if(m_contact_phases.at(i).m_L_init != m_contact_phases.at(i-1).m_L_final){
        std::cout<<"Init AM value do not match final value of previous phase for id : "<<i<<std::endl;
        return false;
      }
      if(m_contact_phases.at(i).m_dL_init != m_contact_phases.at(i-1).m_dL_final){
        std::cout<<"Init AM derivative do not match final AM derivative of previous phase for id : "<<i<<std::endl;
        return false;
      }
    }
    return true;
  }

  /**
   * @brief haveCentroidalValues Check that the initial and final CoM position and AM values are defined for all phases
   * Also check that the initial values of one phase correspond to the final values of the previous ones.
   * @return
   */
  bool haveCentroidalValues() const{
    return haveAMvalues() && haveCOMvalues();
  }

  /**
   * @brief haveConfigurationsValues Check that the initial and final configuration are defined for all phases
   * Also check that the initial values of one phase correspond to the final values of the previous ones.
   * @return
   */
  bool haveConfigurationsValues() const{
    if(m_contact_phases.front().m_q_init.isZero()){
      std::cout<<"Initial configuration not defined."<<std::endl;
      return false;
    }
    for(size_t i = 1 ; i < m_contact_phases.size() ; ++i){
      if(m_contact_phases.at(i).m_q_init.isZero()){
        std::cout<<"Intermediate configuration not defined for phase : "<<i<<std::endl;
        return false;
      }
      if(m_contact_phases.at(i).m_q_init != m_contact_phases.at(i-1).m_q_final){
        std::cout<<"Init configuration do not match final configuration of previous phase for id : "<<i<<std::endl;
        return false;
      }
    }
    if(m_contact_phases.back().m_q_final.isZero()){
      std::cout<<"Final configuration not defined."<<std::endl;
      return false;
    }
    return true;
  }

  /**
   * @brief haveCOMtrajectories check that a c, dc and ddc trajectories are defined for each phases
   * Also check that the time interval of this trajectories matches the one of the phase
   * and that the trajectories start and end and the correct values defined in each phase
   * @return
   */
  bool haveCOMtrajectories() const{
    if(!(haveTimings() && haveCOMvalues()))
      return false;
    size_t i = 0;
    for(const ContactPhase& phase : m_contact_phases){
      if(!phase.m_c){
        std::cout<<"CoM position trajectory not defined for phase : "<<i<<std::endl;
        return false;
      }
      if(!phase.m_dc){
        std::cout<<"CoM velocity trajectory not defined for phase : "<<i<<std::endl;
        return false;
      }
      if(!phase.m_ddc){
        std::cout<<"CoM acceleration trajectory not defined for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_c->min() != phase.timeInitial()){
        std::cout<<"CoM trajectory do not start at t_init for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_dc->min() != phase.timeInitial()){
        std::cout<<"CoM velocity trajectory do not start at t_init for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_ddc->min() != phase.timeInitial()){
        std::cout<<"CoM acceleration trajectory do not start at t_init for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_c->max() != phase.timeFinal()){
        std::cout<<"CoM trajectory do not end at t_final for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_dc->max() != phase.timeFinal()){
        std::cout<<"CoM velocity trajectory do not end at t_final for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_ddc->max() != phase.timeFinal()){
        std::cout<<"CoM acceleration trajectory do not end at t_final for phase : "<<i<<std::endl;
        return false;
      }
      if(!(*phase.m_c)(phase.m_c->min()).isApprox(phase.m_c_init) ){
        std::cout<<"CoM trajectory do not start at c_init for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_dc_init.isZero()){
        if(!(*phase.m_dc)(phase.m_dc->min()).isZero()){
          std::cout<<"CoM velocity trajectory do not start at dc_init for phase : "<<i<<std::endl;
          return false;
        }
      }
      // FIXME : isApprox do not work when values close to 0
      else if(!(*phase.m_dc)(phase.m_dc->min()).isApprox(phase.m_dc_init) ){
        std::cout<<"CoM velocity trajectory do not start at dc_init for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_ddc_init.isZero()){
        if(!(*phase.m_ddc)(phase.m_ddc->min()).isZero() ){
          std::cout<<"CoM velocity trajectory do not start at ddc_init for phase : "<<i<<std::endl;
          return false;
        }
      }
      else if(!(*phase.m_ddc)(phase.m_ddc->min()).isApprox(phase.m_ddc_init) ){
        std::cout<<"CoM acceleration trajectory do not start at ddc_init for phase : "<<i<<std::endl;
        return false;
      }
      if(!(*phase.m_c)(phase.m_c->max()).isApprox(phase.m_c_final) ){
        std::cout<<"CoM trajectory do not end at c_final for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_dc_final.isZero()){
        if(!(*phase.m_dc)(phase.m_dc->max()).isZero() ){
          std::cout<<"CoM velocity trajectory do not end at dc_final for phase : "<<i<<std::endl;
          return false;
        }
      }
      else if(!(*phase.m_dc)(phase.m_dc->max()).isApprox(phase.m_dc_final) ){
        std::cout<<"CoM velocity trajectory do not end at dc_final for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_ddc_final.isZero()){
        if(!(*phase.m_ddc)(phase.m_ddc->max()).isZero() ){
          std::cout<<"CoM velocity trajectory do not end at ddc_final for phase : "<<i<<std::endl;
          return false;
        }
      }
      else if(!(*phase.m_ddc)(phase.m_ddc->max()).isApprox(phase.m_ddc_final) ){
        std::cout<<"CoM velocity trajectory do not end at ddc_final for phase : "<<i<<std::endl;
        return false;
      }
      ++i;
    }
    return true;
  }

  /**
   * @brief haveAMtrajectories check that a L and dL trajectories are defined for each phases
   * Also check that the time interval of this trajectories matches the one of the phase
   * and that the trajectories start and end and the correct values defined in each phase
   * @return
   */
  bool haveAMtrajectories() const{
    if(!(haveTimings() && haveAMvalues()))
      return false;
    size_t i = 0;
    for(const ContactPhase& phase : m_contact_phases){
      if(!phase.m_L){
        std::cout<<"AM position trajectory not defined for phase : "<<i<<std::endl;
        return false;
      }
      if(!phase.m_dL){
        std::cout<<"AM velocity trajectory not defined for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_L->min() != phase.timeInitial()){
        std::cout<<"AM trajectory do not start at t_init for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_dL->min() != phase.timeInitial()){
        std::cout<<"AM derivative trajectory do not start at t_init for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_L->max() != phase.timeFinal()){
        std::cout<<"AM trajectory do not end at t_final for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_dL->max() != phase.timeFinal()){
        std::cout<<"AM derivative trajectory do not end at t_final for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_L_init.isZero()){
        if(!(*phase.m_L)(phase.m_L->min()).isZero()){
          std::cout<<"AM trajectory do not start at L_init for phase : "<<i<<std::endl;
          return false;
        }
      }
      else if(!(*phase.m_L)(phase.m_L->min()).isApprox(phase.m_L_init) ){
        std::cout<<"AM trajectory do not start at L_init for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_dL_init.isZero()){
        if(!(*phase.m_dL)(phase.m_dL->min()).isZero()){
          std::cout<<"AM derivative trajectory do not start at dL_init for phase : "<<i<<std::endl;
          return false;
        }
      }
      else if(!(*phase.m_dL)(phase.m_dL->min()).isApprox(phase.m_dL_init) ){
        std::cout<<"AM derivative trajectory do not start at dL_init for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_L_final.isZero()){
        if(!(*phase.m_L)(phase.m_L->max()).isZero()){
          std::cout<<"AM trajectory do not end at L_final for phase : "<<i<<std::endl;
          return false;
        }
      }
      else if(!(*phase.m_L)(phase.m_L->max()).isApprox(phase.m_L_final) ){
        std::cout<<"AM trajectory do not end at L_final for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_dL_final.isZero()){
        if(!(*phase.m_dL)(phase.m_dL->max()).isZero()){
          std::cout<<"AM derivative trajectory do not end at dL_final for phase : "<<i<<std::endl;
          return false;
        }
      }
      else if(!(*phase.m_dL)(phase.m_dL->max()).isApprox(phase.m_dL_final) ){
        std::cout<<"AM derivative trajectory do not end at dL_final for phase : "<<i<<std::endl;
        return false;
      }
      ++i;
    }
    return true;
  }

  /**
   * @brief haveCentroidalTrajectories check that all centroidal trajectories are defined for each phases
   * Also check that the time interval of this trajectories matches the one of the phase
   * and that the trajectories start and end and the correct values defined in each phase
   * @return
   */
  bool haveCentroidalTrajectories() const{
    return haveAMtrajectories() && haveCOMtrajectories();
  }

  /**
   * @brief haveEffectorsTrajectories check that for each phase preceeding a contact creation,
   *  an SE3 trajectory is defined for the effector that will be in contact.
   * Also check that this trajectory is defined on the time-interval of the phase.
   * Also check that the trajectory correctly end at the placement defined for the contact in the next phase.
   * If this effector was in contact in the previous phase, it check that the trajectory start at the previous contact placement.
   * @return
   */
  bool haveEffectorsTrajectories() const{
    if(!haveTimings())
      return false;
    for(size_t i = 0 ; i < m_contact_phases.size() -1 ; ++i){
      for(std::string eeName : m_contact_phases.at(i).getContactsCreated(m_contact_phases.at(i+1))){
        if(! m_contact_phases.at(i).effectorHaveAtrajectory(eeName)){
          std::cout<<"No end effector trajectory for "<<eeName<<" at phase "<<i<<" but it is in contact at phase "<<i+1<<std::endl;
          return false;
        }
        const typename ContactPhase::curve_SE3_ptr traj = m_contact_phases.at(i).effectorTrajectories().at(eeName);
        if(traj->min() != m_contact_phases.at(i).timeInitial()){
          std::cout<<"Effector trajectory for "<<eeName<<" do not start at t_init in phase "<<i<<std::endl;
          return false;
        }
        if(traj->max() != m_contact_phases.at(i).timeFinal()){
          std::cout<<"Effector trajectory for "<<eeName<<" do not end at t_final in phase "<<i<<std::endl;
          return false;
        }
        ContactPatch::SE3 pMax = ContactPatch::SE3((*traj)(traj->max()).matrix());
        if(!pMax.isApprox(m_contact_phases.at(i+1).contactPatches().at(eeName).placement())){
          std::cout<<"Effector trajectory for "<<eeName
                  << " do not end at it's contact placement in the next phase, for phase "<<i<<std::endl;
          return false;
        }
        if(i > 0 && m_contact_phases.at(i-1).isEffectorInContact(eeName)){
          ContactPatch::SE3 pMin = ContactPatch::SE3((*traj)(traj->min()).matrix());
          if(!pMin.isApprox(m_contact_phases.at(i-1).contactPatches().at(eeName).placement())){
            std::cout<<"Effector trajectory for "<<eeName
                    << " do not start at it's contact placement in the previous phase, for phase "<<i<<std::endl;
            return false;
          }
        }
      }
    }
    return true;
  }

  /**
   * @brief haveJointsTrajectories Check that a q, dq and ddq trajectories are defined for each phases
   * Also check that the time interval of this trajectories matches the one of the phase
   * and that the trajectories start and end and the correct values defined in each phase
   * @return
   */
  bool haveJointsTrajectories() const{
    if(!(haveTimings() && haveConfigurationsValues()))
      return false;
    size_t i = 0;
    for(const ContactPhase& phase : m_contact_phases){
      if(!phase.m_q){
        std::cout<<"joint position trajectory not defined for phase : "<<i<<std::endl;
        return false;
      }
      if(!phase.m_dq){
        std::cout<<"joint velocity trajectory not defined for phase : "<<i<<std::endl;
        return false;
      }
      if(!phase.m_ddq){
        std::cout<<"joint acceleration trajectory not defined for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_q->min() != phase.timeInitial()){
        std::cout<<"joint trajectory do not start at t_init for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_dq->min() != phase.timeInitial()){
        std::cout<<"joint velocity trajectory do not start at t_init for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_ddq->min() != phase.timeInitial()){
        std::cout<<"joint acceleration trajectory do not start at t_init for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_q->max() != phase.timeFinal()){
        std::cout<<"joint trajectory do not end at t_final for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_dq->max() != phase.timeFinal()){
        std::cout<<"joint velocity trajectory do not end at t_final for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_ddq->max() != phase.timeFinal()){
        std::cout<<"joint acceleration trajectory do not end at t_final for phase : "<<i<<std::endl;
        return false;
      }
      if(!(*phase.m_q)(phase.m_q->min()).isApprox(phase.m_q_init) ){
        std::cout<<"joint trajectory do not start at q_init for phase : "<<i<<std::endl;
        return false;
      }
      if(!(*phase.m_q)(phase.m_q->max()).isApprox(phase.m_q_final) ){
        std::cout<<"joint trajectory do not end at q_final for phase : "<<i<<std::endl;
        return false;
      }
      ++i;
    }
    return true;
  }

  /**
   * @brief haveJointsTrajectories Check that a joint torque trajectories are defined for each phases
   * Also check that the time interval of this trajectories matches the one of the phase
   * and that the trajectories start and end and the correct values defined in each phase
   * @return
   */
  bool haveTorquesTrajectories() const{
    if(!haveTimings())
      return false;
    size_t i =0;
    for(const ContactPhase& phase : m_contact_phases){
      if(!phase.m_tau){
        std::cout<<"Torque trajectory not defined for phase : "<<i<<std::endl;
        return false;
      }

      if(phase.m_tau->min() != phase.timeInitial()){
        std::cout<<"Torque trajectory do not start at t_init for phase : "<<i<<std::endl;
        return false;
      }
      if(phase.m_tau->max() != phase.timeFinal()){
        std::cout<<"Torque trajectory do not end at t_final for phase : "<<i<<std::endl;
        return false;
      }
      ++i;
    }
    return true;
  }

  /**
   * @brief haveJointsTrajectories Check that a contact force trajectory exist for each active contact
   * Also check that the time interval of this trajectories matches the one of the phase
   * and that the trajectories start and end and the correct values defined in each phase
   * @return
   */
  bool haveContactForcesTrajectories() const{
    if(!haveTimings())
      return false;
    size_t i = 0;
    for(const ContactPhase& phase : m_contact_phases){
      for(std::string eeName : phase.effectorsInContact()){
        if(phase.contactForces().count(eeName) == 0){
          std::cout<<"No contact forces trajectory for effector "<<eeName<<" at phase "<<i<<std::endl;
          return false;
        }
        if(phase.contactNormalForces().count(eeName) == 0){
          std::cout<<"No contact normal force trajectory for effector "<<eeName<<" for phase "<<i<<std::endl;
          return false;
        }
        if(phase.contactForces().at(eeName)->min() != phase.timeInitial()){
          std::cout<<"No contact forces trajectory for effector "<<eeName<<" do not start at t_init for phase "<<i<<std::endl;
          return false;
        }
        if(phase.contactForces().at(eeName)->max() != phase.timeInitial()){
          std::cout<<"No contact forces trajectory for effector "<<eeName<<" do not end at t_final for phase "<<i<<std::endl;
          return false;
        }
        if(phase.contactNormalForces().at(eeName)->min() != phase.timeInitial()){
          std::cout<<"No contact normal force trajectory for effector "<<eeName<<" do not start at t_init for phase "<<i<<std::endl;
          return false;
        }
        if(phase.contactNormalForces().at(eeName)->max() != phase.timeInitial()){
          std::cout<<"No contact normal force trajectory for effector "<<eeName<<" do not end at t_final for phase "<<i<<std::endl;
          return false;
        }
      }
      ++i;
    }
    return true;
  }

  /**
   * @brief getAllEffectorsInContact return a vector of names of all the effectors used to create contacts during the sequence
   * @return
   */
  t_strings getAllEffectorsInContact() const{
    // use set to guarantee uniqueness, but return a vector for easier use and python bindings
    std::set<std::string> res_set;
    for(const ContactPhase& phase : m_contact_phases){
      for(const std::string& eeName : phase.effectorsInContact()){
        res_set.insert(eeName);
      }
    }
    return t_strings(res_set.begin(), res_set.end());
  }

  /* End Helpers */

  /*Public Attributes*/
  ContactPhaseVector m_contact_phases;
  /*Public Attributes*/

 private:
  // Serialization of the class
  friend class boost::serialization::access;

  template <class Archive>
  void save(Archive& ar, const unsigned int /*version*/) const {
    const size_t m_size = size();
    ar& boost::serialization::make_nvp("size", m_size);
    for (typename ContactPhaseVector::const_iterator it = m_contact_phases.begin(); it != m_contact_phases.end();
         ++it) {
      ar& boost::serialization::make_nvp("contact_phase", *it);
    }
  }

  template <class Archive>
  void load(Archive& ar, const unsigned int /*version*/) {
    size_t m_size;
    ar >> boost::serialization::make_nvp("size", m_size);
    assert(m_size > 0);
    resize(m_size);
    for (typename ContactPhaseVector::iterator it = m_contact_phases.begin(); it != m_contact_phases.end(); ++it) {
      ar >> boost::serialization::make_nvp("contact_phase", *it);
    }
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()

};  // end class ContactSequence

}  // namespace scenario
}  // namespace multicontact_api
#endif  // __multicontact_api_scenario_contact_sequence_hpp__
