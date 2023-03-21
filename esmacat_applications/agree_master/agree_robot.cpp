//
// Author:  Stefano Dalla Gasperina
//          stefano.dallagasperina@polimi.it

#include "agree_robot.h"

using namespace std;

// ////////////////////////////////////////////////////////////////
// //////////////// MOTOR CONTROLLER POINTERS /////////////////////
/// \brief agree_robot::assign_esmacat_slave_index
/// \param slave_in_app
/// \param slave_index
/// \return
///
void agree_robot_class::assign_esmacat_slave_index(agree_motor_controller* slave_in_app, int slave_index){
    agree_joints[slave_index] = slave_in_app;
    return;
}

// ////////////////////////////////////////////////////////////////
// //////////////// ROBOT CONSTRUCTOR /////////////////////////////
/// \brief Robot::Robot
///
agree_robot_class::agree_robot_class(){

    // Joint Sign
    joint_sign.resize(N_DOFS_MAX);

    // Computed Variables
    gravity_torques_Nm.resize(N_DOFS_MAX);
    signed_gravity_torques_mNm.resize(N_DOFS_MAX);
    weight_compensation_torques_Nm.resize(N_DOFS_MAX);
    signed_weight_compensation_torques_mNm.resize(N_DOFS_MAX);

    // Measured Variables
    position_rad.resize(N_DOFS_MAX);
    velocity_rad_s.resize(N_DOFS_MAX);
    loadcell_torque_mNm.resize(N_DOFS_MAX);
    impedance_control_setpoint_rad.resize(N_DOFS_MAX);
    impedance_control_k_gain_mNm_per_rad.resize(N_DOFS_MAX);
    impedance_control_d_gain_mNm_per_rad_per_sec.resize(N_DOFS_MAX);

    // Set all joint to false
    joint_is_referenced = Eigen::Array<bool,1,N_DOFS_MAX>::Zero(N_DOFS_MAX);
    robot_is_ready = false;
}

// ////////////////////////////////////////////////////////////////
// //////////////// GET PARAMETERS ////////////////////////////////
/// \brief agree_robot::read_agree_system_parameters
///
void agree_robot_class::read_agree_robot_parameters(int side){

    // side = 0 -> Right
    // side = 1 -> Left

    // Human anthropometric data
    human_mass_kg     = 0;       // [kg]
    human_height_m    = 1.80;     // [m]

    // Trunk lengths in meters
    trunk_height_m    = 0.800;
    trunk_width_m     = 0.200;

    // Memory Allocation
    for (int i=0;i<N_FRAMES;i++){
        link_mass_kg[i]          =  0;
        arm_mass_kg[i]           =  0;
        link_com_m[i]            <<  0,0,0;
        arm_com_m[i]             <<  0,0,0;
        I_tensor[i]              <<  0,0,0,0,0,0,0,0,0;
    }

    // Right Side
    if( side == RIGHT) {
        // Position of i-th center of mass (X,Y,Z) wrt i-th Frame
        link_com_m[0]  <<  0.000, 0.000, 0.000;           // Base
        link_com_m[1]  <<  0.000, 0.000, 0.000;           // Inertial
        link_com_m[2]  <<  0.000, -93.0, 50.00;           // Link 1
        link_com_m[3]  <<  -23.0, -62.0, 104.0;           // Link 2
        link_com_m[4]  <<  -76.0   , -11.0  ,    -30.0;   // Link 3
        link_com_m[5]  <<  -49.0   , - 87.0   ,   5.0;    // Link 4
        link_com_m[6]  <<  -22.0  ,  20.0  ,   -73.0;     // Link 5
        link_com_m[7]  <<  0.000, 0.000, 0.000;           // EE
        // Joint sign between DH model and positive rotation of motor
        joint_sign << 1, -1, -1, -1, 0;

    }
    else {
        // Position of i-th center of mass (X,Y,Z) wrt i-th Frame
        link_com_m[0]  <<  0.000, 0.000, 0.000;       // Base
        link_com_m[1]  <<  0.000, 0.000, 0.000;       // Inertial
        link_com_m[2]  <<  0.000, 93.0, 50.00; // Link 1
        link_com_m[3]  <<  -23.0, 62.0, 104.0; // Link 2
        link_com_m[4]  <<  -76.0   , 11.0  ,    -30.0;  // Link 3
        link_com_m[5]  <<  -49.0   , 87.0   ,   5.0;  // Link 4
        link_com_m[6]  <<  -22.0  ,  -20.0  ,   -73.0; // Link 5
        link_com_m[7]  <<  0.000, 0.000, 0.000;       // EE
        // Joint sign between DH model and positive rotation of motor
        joint_sign << -1, 1, 0, 1, 0;

    }

    // Conversion from mm to m.
    for(int i=0;i<N_FRAMES;i++){
        link_com_m[i] /= 1000;
    }

    // Mass of i-th link [kg]
    // from CAD design
    link_mass_kg[0] = 0;    // Base
    link_mass_kg[1] = 0;    // Inertial
    link_mass_kg[2] = 557;    // Link J1
    link_mass_kg[3] = 2276;    // Link J2
    link_mass_kg[4] = 1721;    // Link J3
    link_mass_kg[5] = 1028;    // Link J4
    link_mass_kg[6] = 453;    // Link J5
    link_mass_kg[7] = 0;    // Link J6

    // Conversion from g to kg
    for(int i=0;i<N_FRAMES;i++)
    {
        link_mass_kg[i] /= 1000;
    }

    /** Arm Weight Compensation Parameters **/

    // Arm anthropometric lengths in meters
    // Winter et al. 2009
    upperarm_length_m = 0.186*human_height_m; // 0.242;
    lowerarm_length_m = 0.146*human_height_m; // 0.270;
    hand_length_m     = 0.108*human_height_m;

    // Mass of i-th human body parts [kg]
    // from Winter et al. 2009
    arm_mass_kg[3] = 0.028*human_mass_kg; // Upperarm
    arm_mass_kg[5] = 0.016*human_mass_kg; // Lowerarm
    arm_mass_kg[6] = 0.006*human_mass_kg; // Hand

    // Position of center of mass [m]
    // Right Side
    if( side == RIGHT) {
        arm_com_m[3] << 0,-0.436*upperarm_length_m,0;
        arm_com_m[5] << 0,-0.43 *lowerarm_length_m,0;
        arm_com_m[6] << 0,0,0.506*hand_length_m;

    }
    // Left Side
    else if( side == LEFT) {
        arm_com_m[3] << 0,0.436*upperarm_length_m,0;
        arm_com_m[5] << 0,0.43 *lowerarm_length_m,0;
        arm_com_m[6] << 0,0,0.506*hand_length_m;
    }

    // Load Denavit-Hartenberg model
    get_dh_model(side);

}

// ////////////////////////////////////////////////////////////////
// //////////////// DENAVIT-HARTENBERG ////////////////////////////
/// \brief agree_robot::get_dh_mode
/// \return
///
Eigen::MatrixXd agree_robot_class::get_dh_model(int side)
{
    Eigen::MatrixXd dh_table;

    double h  = trunk_height_m;
    double w  = trunk_width_m;
    double lh = upperarm_length_m;
    double lf = lowerarm_length_m;

    //Memory allocation
    dh_table = Eigen::MatrixXd::Zero(N_FRAMES,4);

    // Right Side
    if(side == RIGHT) {
        //            offset(deg),     d ,     a ,   alpha(deg)
        dh_table.row(0) <<    0.0,    0.0,    0.0,    0.0;  // Base
        dh_table.row(1) <<    0.0,      h,      w,    0.0;  // Inertial
        dh_table.row(2) <<   90.0,    0.0,    0.0,   90.0;  // J1
        dh_table.row(3) <<   90.0,    0.0,    0.0,   90.0;  // J2
        dh_table.row(4) <<  -90.0,     lh,    0.0,  -90.0;  // J3
        dh_table.row(5) <<    0.0,    0.0,    0.0,   90.0;  // J4
        dh_table.row(6) <<    0.0,     lf,    0.0,    0.0;  // J5
        dh_table.row(7) <<    0.0,    0.0,    0.0,    0.0;  // EE
    }
    // Left side
    else if (side == LEFT){
        //            offset(deg),     d ,     a ,   alpha(deg)
        dh_table.row(0) <<    0.0,    0.0,    0.0,    0.0;  // Base
        dh_table.row(1) << -180.0,      h,      w,    0.0;  // Inertial
        dh_table.row(2) <<  -90.0,    0.0,    0.0,  -90.0;  // J1
        dh_table.row(3) <<  -90.0,    0.0,    0.0,  -90.0;  // J2
        dh_table.row(4) <<   90.0,     lh,    0.0,   90.0;  // J3
        dh_table.row(5) <<    0.0,    0.0,    0.0,  -90.0;  // J4
        dh_table.row(6) <<    0.0,     lf,    0.0,    0.0;  // J5
        dh_table.row(7) <<    0.0,    0.0,    0.0,    0.0;  // EE
    }

    // Conversion to radians
    dh_table.col(0) *= M_PI/180;
    dh_table.col(3) *= M_PI/180;

    // Save matrix to class variable
    dh_model = dh_table;

    //Return matrix
    return dh_table;
}


// ////////////////////////////////////////////////////////////////
// //////////////// KINEMATICS and JACOBIAN ///////////////////////
/// \brief agree_robot_class::direct_kinematics
///
void agree_robot_class::direct_kinematics(){

    //Transformation Frames
    Frame Tz, Rz, Tx, Rx;

    //Expanded vector to include base,inertial and end-effector frames
    Eigen::VectorXd theta(N_FRAMES);
    theta <<            0,                              // Base
                        0,                              // Inertial
                        position_rad(0)*joint_sign(0),  // J1 Shoulder
                        position_rad(1)*joint_sign(1),  // J2 Shoulder
                        position_rad(2)*joint_sign(2),  // J3 Shoulder
                        position_rad(3)*joint_sign(3),  // J4 Elbow
                        position_rad(4)*joint_sign(4),  // J5 Wrist
                        0;                              // EE

    // TODO: J3 in fixed at 0°
    // Used on V1, fixed J3 at about 45°
    theta(4) += deg_to_rad*(0.0);

    //Definition of gravity (reverse sign is used to compute anti-gravity torques)
    g_hat[0] = translation(9.81,'z');

    // Propagate transformations with DH convention
    for(int i = 1; i < N_FRAMES; ++i)
    {
        // Build Rx,Tx,Rz,Tz (Modified DH, John Craig convention)
        // Rz: DH Offsets are added to virtual_theta angles
        Rx = htm(dh_model(i-1, 3), 'x', 0.0, 0);
        Tx = htm(0.0, 0, dh_model(i-1, 2), 'x');
        Rz = htm(dh_model(i, 0) + theta(i),'z', 0.0, 0);
        Tz = htm(0.0, 0, dh_model(i, 1), 'z');

        // Concatenate Transformations
        T_matrix[i] = Rx*Tx*Rz*Tz;
        // Compute Pose Transformation matrix at each frame
        T_total[i] = T_total[i-1]*T_matrix[i];
        // Compute Gravity Force at each frame
        g_hat[i] = T_total[i].orientation.transpose()*g_hat[0];
    }
}

// ////////////////////////////////////////////////////////////////
// ////////////////////// GET JACOBIAN ////////////////////////////
/// \brief agree_robot::get_Jacobian
///
void agree_robot_class::get_Jacobian(){
    Eigen::Vector3d J_i[N_FRAMES];
    Eigen::Vector3d Z_hat = translation(1,'z');
    Eigen::MatrixXd J(J_i[1].rows(),N_DOFS);

    // Siciliano 3.1.3 Jacobian Computation or
    // John Craig (Eq. 5.110)
    for (int i=1 ; i<N_FRAMES; i++)
    {
        // Compute Jacobian Matrix for each frame
        J_i[i]=(T_total[i-1].orientation*Z_hat).cross(T_total[N_FRAMES-1].position - T_total[i-1].position );
    }

    // Extract active joints
    J << J_i[3], J_i[4], J_i[5], J_i[6], J_i[7];

    Jacobian = J;
}

// ////////////////////////////////////////////////////////////////
// //////////////// GRAVITY COMPENSATION //////////////////////////
/// \brief agree_robot::get_inverse_dynamics_torques
/// \return
///
Eigen::VectorXd agree_robot_class::get_inverse_dynamics_torques(){

    //Joint frame i is the htm from i-1 to i
    Eigen::VectorXd virtual_tau(N_FRAMES);
    Eigen::VectorXd tau(5);
    Eigen::VectorXd theta_dot(N_FRAMES);
    Eigen::VectorXd theta_ddot(N_FRAMES);

    Eigen::Vector3d  omega[N_FRAMES], omega_dot[N_FRAMES] , v_dot_hat[N_FRAMES],vc_dot[N_FRAMES], f_hat[N_FRAMES] , n_hat[N_FRAMES], F_internal[N_FRAMES], N_internal[N_FRAMES],F_external[N_FRAMES], N_external[N_FRAMES],F_external_link_position[N_FRAMES] ;

    //Vector of Z axis within the respective joint frame. Always equal to (0,0,1)
    Eigen::Vector3d Z_hat = translation(1,'z');

    //Get values of required quantities. Currently set to zero since we do not calculate inertia dynamics
    for(int i = 0; i < N_FRAMES; ++i){
        F_external[i]               << 0.0,0.0,0.0;
        N_external[i]               << 0.0,0.0,0.0;
        F_external_link_position[i] << 0.0,0.0,0.0;
    }

    theta_dot   <<  0,
                    0,
                    velocity_rad_s(0)*joint_sign(0),
                    velocity_rad_s(1)*joint_sign(1),
                    velocity_rad_s(2)*joint_sign(2),
                    velocity_rad_s(3)*joint_sign(3),
                    velocity_rad_s(4)*joint_sign(4),
                    0;

    theta_ddot  << 0,0,0,0,0,0,0,0;

    //Initialization
    omega[0]        << 0.0,0.0,0.0;
    omega_dot[0]    << 0.0,0.0,0.0;
    v_dot_hat[0]    << 0.0,0.0,0.0;
    vc_dot[0]       << 0.0,0.0,0.0;
    f_hat[N_FRAMES] << 0.0,0.0,0.0;
    n_hat[N_FRAMES] << 0.0,0.0,0.0;

    // Outward iteration (Craig 6.45-6.50)
    // Note that we use the translose of the Rotation matrices since the inverse is equal to the transpose
    for(int i = 0; i < N_FRAMES-1; ++i){

        //(Eq 6.45)
        omega[i+1]  = T_matrix[i+1].orientation.transpose()*omega[i] + theta_dot[i+1]*Z_hat;

        //(Eq 6.46)
        omega_dot[i+1]  = T_matrix[i+1].orientation.transpose()*omega_dot[i]
                + (T_matrix[i+1].orientation.transpose()*omega[i]).cross(theta_dot[i+1]*Z_hat)
                + (theta_ddot[i+1]*Z_hat);

        //(Eq 6.47)
        v_dot_hat[i+1]      = T_matrix[i+1].orientation.transpose()
                * (omega_dot[i].cross(T_matrix[i+1].position)
                + omega[i].cross(omega[i].cross(T_matrix[i+1].position))
                + v_dot_hat[i]);

        //(Eq 6.48)
        vc_dot[i+1]  = omega_dot[i+1].cross(link_com_m[i+1])
                + omega[i+1].cross(omega[i+1].cross(link_com_m[i+1]))
                + v_dot_hat[i+1];

        // Setting values to zero to eliminate dynamics from the output leaving only gravity comp
        //        omega_hat[i+1] << 0.0,0.0,0.0;
        //        alpha_hat[i+1] << 0.0,0.0,0.0;
        //        a_hat[i+1]    << 0.0,0.0,0.0;
        //        a_hat_CG[i+1] << 0.0,0.0,0.0;
        //        moment_internal_hat[i+1] << 0.0,0.0,0.0;

        //(Eq 6.49)
        F_internal[i+1] = link_mass_kg[i+1]*vc_dot[i+1]
                        + link_mass_kg[i+1]*g_hat[i+1];

        // Vertical External force due to arm weight
        F_external[i+1] = arm_mass_kg[i+1]*g_hat[i+1];

        //(Eq 6.50)
        N_internal[i+1] = I_tensor[i+1]*omega_dot[i+1]
                + omega[i+1].cross(I_tensor[i+1]*omega[i+1]);
    }

    // Inward iteration  (Craig 6.51-6.53)
    for(int i=N_FRAMES-1; i>=0; i--){

        // (Eq 6.51)
        f_hat[i] =
                T_matrix[i+1].orientation*f_hat[i+1]
                + F_internal[i]
                + F_external[i]
                ;

        // (Eq 6.52)
        n_hat[i] = N_internal[i]
                + T_matrix[i+1].orientation*n_hat[i+1]
                + link_com_m[i].cross(F_internal[i])
                + arm_com_m[i].cross(F_external[i])
                + T_matrix[i+1].position.cross(T_matrix[i+1].orientation*f_hat[i+1])
                + N_external[i+1]
                ;

        virtual_tau(i)= n_hat[i].transpose()*Z_hat;

    }

    // Spring Counterweight
    /** TODO: */
    //spring counterweight
    //        double x= J[2]->get_incremental_enc_radians();
    //        double spring_counterweight_Nm = 0.5*(34.74294 + 0.3760517*x - 0.01475961*pow(x,2) + 0.00007924522*pow(x,3));//y = 34.74294 + 0.3760517*x - 0.01475961*x^2 + 0.00007924522*x^3
    //        gravity_torque_mNm[2]=gravity_torque_mNm[2]- spring_counterweight_mNm;

    // Compute active torques vector
    tau  <<  virtual_tau(2), // J1
            virtual_tau(3), // J2 dynamics are affected by the spring counterweight
            virtual_tau(4), // J3
            virtual_tau(5), // J4
            virtual_tau(6); // J5

    gravity_torques_Nm = tau;

    for(int joint_index=0;joint_index<N_DOFS_MAX;joint_index++){
       signed_gravity_torques_mNm(joint_index) = Nm_to_mNm*gravity_torques_Nm(joint_index)*joint_sign(joint_index); //Gravity torque taking into account the signs on the different joints
    }

    return gravity_torques_Nm;
}

// ////////////////////////////////////////////////////////////////
// //////////////// GRAVITY COMPENSATION //////////////////////////
/// \brief agree_robot_class::get_inverse_dynamics_torque_onedof
/// \param weight_assistance
/// \param position
/// \return
///
double agree_robot_class::get_inverse_dynamics_torque_onedof(float weight_assistance, double position){
    double gravity_torque_mNm;
    double weight_compensation_mNm;

    double link_com    = 0.140;
    double link_mass   = 0.350;

    double shell_com   = 0.170;
    double shell_mass  = 0.220;

    double human_mass =     55;
    double human_height = 1.70;

    double forearm_mass = 0.016*human_mass;
    double forearm_length = 0.146*human_height;
    double forearm_com  = 0.43*forearm_length;

    double hand_mass = 0.006*human_mass;
    double hand_length = 0.108*human_height;
    double hand_com = 0.506*hand_length;

    gravity_torque_mNm  = link_mass*9.81*sin(position)*link_com*1000 +
                        + shell_mass*9.81*sin(position)*(shell_com)*1000   ;

    weight_compensation_mNm = forearm_mass*9.81*sin(position)*forearm_com*1000 +
                            + hand_mass*9.81*sin(position)*(forearm_length+hand_com)*1000;

    return gravity_torque_mNm + weight_assistance*weight_compensation_mNm;
}


// ////////////////////////////////////////////////////////////////
// //////////////// FRICTION COMPENSATION //////////////////////////
/// \brief agree_robot_class::friction_compensation_torque
/// \return
///
Eigen::VectorXd agree_robot_class::friction_compensation_torque(){

    Eigen::VectorXd friction_tau(N_DOFS);
    Eigen::MatrixXd B;

    // Build friction matrix for 2-bar linkage
    B = Eigen::MatrixXd::Identity(N_DOFS,N_DOFS);
    B *= 0.005;

    // Build friction torque
    friction_tau = B*velocity_rad_s;

    return friction_tau;
}

///
/// \brief agree_robot_class::get_current_joint_angles_rad
/// \return
///
Eigen::VectorXd agree_robot_class::get_current_joint_angles_rad()
{
    Eigen::VectorXd joint_angles_rad(N_DOFS_MAX);
    for (int i=0;i<N_DOFS;i++){
        agree_joints[i]->position_rad = agree_joints[i]->get_encoder_position_radians();
        joint_angles_rad(i)           = agree_joints[i]->position_rad;
    }
    for (int i=N_DOFS;i<N_DOFS_MAX;i++){
        joint_angles_rad(i) = 50.0/180.0*M_PI;

    }
    return joint_angles_rad;
}

///
/// \brief agree_robot_class::get_current_joint_velocity_rad
/// \return
///
Eigen::VectorXd agree_robot_class::get_current_joint_velocity_rad_s()
{
    Eigen::VectorXd joint_velocity_rad_s(N_DOFS_MAX);
    for (int i=0;i<N_DOFS;i++){
        agree_joints[i]->velocity_rad_per_s = agree_joints[i]->get_encoder_filt_speed_radians();
        joint_velocity_rad_s(i) = agree_joints[i]->get_encoder_filt_speed_radians();
    }
    for (int i=N_DOFS;i<N_DOFS_MAX;i++){
        joint_velocity_rad_s(i) = 0.0/180.0*M_PI;
    }
    return joint_velocity_rad_s;
}

///
/// \brief agree_robot_class::get_current_loadcell_torque_mNm
/// \return joint loadcell torque in milli-Nm
///
Eigen::VectorXd agree_robot_class::get_current_loadcell_torque_mNm()
{
    Eigen::VectorXd joint_torque_mNm(N_DOFS_MAX);
    for (int i=0;i<N_DOFS;i++){
        joint_torque_mNm(i) = agree_joints[i]->get_loadcell_torque_mNm();
    }
    for (int i=N_DOFS;i<N_DOFS_MAX;i++){
        joint_torque_mNm(i) = 0.0;
    }
    return joint_torque_mNm;
}


esmacat_err agree_robot_class::get_epos4_errorcode(){

    esmacat_err error = NO_ERR;
    int errorcode = 0;

    for (int i=0;i<N_DOFS;i++){
        errorcode = agree_joints[i]->get_errorcode();
        if(errorcode != 0)
        {
            std::stringstream ss;
            ss << std::hex << errorcode; // int decimal_value
            std::string res ( ss.str() );
            PLOGE << "EPOS4 Error J" << i+1 << " CODE: 0x" << res;
            error = ERR_UNKNOWN;
        }
    }
    return error;
}


///
/// \brief agree_robot_class::reset_joint_angles
///
void agree_robot_class::reset_joint_angles()
{
    for (int joint_index=0;joint_index<N_DOFS;joint_index++)
    {
        agree_joints[joint_index]->clear_encoder_counter();
    }
}

///
/// \brief agree_robot_class::reset_fault
///
void agree_robot_class::reset_fault()
{
    for (int joint_index=0;joint_index<N_DOFS;joint_index++)
    {
        agree_joints[joint_index]->reset_fault();
    }
}

///
/// \brief agree_robot_class::start_motor
///
void agree_robot_class::start_motor()
{
    for (int joint_index=0;joint_index<N_DOFS;joint_index++)
    {
        agree_joints[joint_index]->start_motor();
    }
}

///
/// \brief agree_robot_class::stop_motor
///
void agree_robot_class::stop_motor()
{
    for (int joint_index=0;joint_index<N_DOFS;joint_index++)
    {
        agree_joints[joint_index]->stop_motor();
    }
}

///
/// \brief agree_robot_class::set_mode_operation
/// \param mode of operation of EPOS4
///
void agree_robot_class::set_mode_operation(int mode)
{
    for (int joint_index=0;joint_index<N_DOFS;joint_index++)
    {
        agree_joints[joint_index]->set_mode_operation(mode);
    }
}

///
void agree_robot_class::set_elapsed_time_ms(double elapsed_time_ms){
    for (int joint_index=0;joint_index<N_DOFS;joint_index++)
    {
        agree_joints[joint_index]->elapsed_time = elapsed_time_ms;
    }
}

///
/// \brief agree_robot_class::save_joint_status
///
void agree_robot_class::save_joint_status(){
    for (int joint_index=0;joint_index<N_DOFS;joint_index++)
    {
        agree_joints[joint_index]->save_joint_status();
    }
}

///
/// \brief agree_robot_class::set_impedance_control_gain
///
void agree_robot_class::set_impedance_control_gain(){
    for (int joint_index=0;joint_index<N_DOFS;joint_index++)
    {
        agree_joints[joint_index]->set_impedance_control_gain(agree_joints[joint_index]->joint_task_control_command.impedance_control_k_gain_mNm_per_rad,
                                                              agree_joints[joint_index]->joint_task_control_command.impedance_control_d_gain_mNm_per_rad_per_sec);
    }
}

void agree_robot_class::run_impedance_control(){
    for (int joint_index=0;joint_index<N_DOFS;joint_index++)
    {
        agree_joints[joint_index]->set_impedance_control_rad(impedance_control_setpoint_rad[joint_index],-gravity_torques_Nm[joint_index]);
    }
}

///
////// \brief agree_robot_class::run_homing_mode
/// This function runs Homing-Mode on all joints and sets joint_is_referenced flag to true when Homing is completed on all joints.
///
void agree_robot_class::run_homing_mode(){
    // Start homing mode on all active joints.
    // The function set_homing_mode returns true when joint encoder is referenced.
    for (int joint_index=0;joint_index<N_DOFS;joint_index++){
        joint_is_referenced[joint_index] = agree_joints[joint_index]->set_homing_mode();
    }
    // For all non-active joints set joint_is_referenced flag to true.
    for (int joint_index=N_DOFS;joint_index<N_DOFS_MAX;joint_index++){
        joint_is_referenced[joint_index] = true;
    }
}
