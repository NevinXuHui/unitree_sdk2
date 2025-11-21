# LowCmd 发布者
add_executable(lowcmd_publisher 
    lowcmd_publisher.cpp
)
target_link_libraries(lowcmd_publisher unitree_sdk2)

# LowCmd 订阅者
add_executable(lowcmd_subscriber 
    lowcmd_subscriber.cpp
)
target_link_libraries(lowcmd_subscriber unitree_sdk2)
