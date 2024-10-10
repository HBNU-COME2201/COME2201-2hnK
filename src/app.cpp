#include <iostream>
#include "agent.hpp"
#include "mmanager.hpp"
#include "stochastic_decorator.hpp"
#include "simple_decorator.hpp"

#include <vector>
#include <string>
#include "tinyxml2.h"
#include "special_agent.hpp"

/*
[학습 목표]
기존의 코드는 agent를 추가할 때마다 client 코드를 컴파일해야 한다.

수정된 코드에서는 객체의 생성 및 수정을 client 코드 내에서 하지 않고,
제공하는 API를 통해 객체를 생성할 수 있도록 만든다.
*/
class InitManager
{
public:
    InitManager() {}              // default 생성자
    InitManager(std::string path) // string을 입력받는 생성자
    {
        tinyxml2::XMLDocument doc;
        doc.LoadFile(path.c_str()); // c_str: std::string 클래스에서 제공하는 메서드로, C 스타일의 문자열을 반환한다.

        /*
        [XML 요소들]
        <Agent x="10"> abc </Agent>
            <Agent>...</Agent>: element
            x="10": attribute
            abc: text

        [XMLDocument 메서드]
        doc.FirstChildElement("scenario"):
        doc("test.xml")에 존재하는 <scenario> 태그의 첫 번째 자식 요소(ChildElement)를 불러온다. → <scenario>

        -> FirstChildElement("AgentList"):
        위에서 불러온 <AgentList> 태그의 첫 번째 ChildElement를 불러온다. → <AgentList>

        agListElem -> FirstChildElement():
        <AgentList> 태그의 첫 번째 자식 요소 <Agent>를 불러온다. → <Agent>

        agElem -> NextSiblingElement():
        현재 <Agent> 태그의 다음 형제 요소를 불러온다.

        [Clean Room Approach]
        정상적으로 작동되는 부분을 넓혀 나아가는 방식.
        완성된 코드에 추가된 부분을 테스트 → 검토하여 문제가 없음을 추가된 코드에 대해 확인한다.
        */
        tinyxml2::XMLElement *agListElem = doc.FirstChildElement("scenario")->FirstChildElement("AgentList");

        double x, y, heading, speed, drange;
        for (tinyxml2::XMLElement *agElem = agListElem->FirstChildElement();
             agElem != NULL; agElem = agElem->NextSiblingElement())
        {
            /*
            변수 double x, y, heading, speed, drange에
            agElem의 attribute x, y, heading, speed, drange값을 대입한다.
            */
            agElem->QueryDoubleAttribute("x", &x);
            agElem->QueryDoubleAttribute("y", &y);
            agElem->QueryDoubleAttribute("heading", &heading);
            agElem->QueryDoubleAttribute("speed", &speed);
            agElem->QueryDoubleAttribute("drange", &drange);

            CAgent *ag = new CSpecialAgent(x, y, heading, speed, drange);

            m_agent_list.push_back(ag); // 벡터의 맨 뒤에 CAgent* ag를 추가
        }
    }

private:
    std::vector<CAgent *> m_agent_list;
    /*
    std::vector: 동적 배열 컨테이너
    <CAgent *>: 배열에 CAgent 객체의 포인터를 저장한다
    */

public:
    std::vector<CAgent *> &get_agent_list() { return m_agent_list; }
    /*
    [객체가 아닌 포인터를 반환하는 이유]
    객체를 직접적으로 반환하면 동일한 값의 새로운 객체를 생성하기 때문에 성능이 떨어지기 때문이다.
    */
};

int main(int argc, char **argv)
{ // Clinet Code

    InitManager init_manager("test.xml");
    /* Simulation Engine */
    
    /*
    [다형성의 특성]
    SpecialAgent는 CAgent를 상속받는 자식 클래스이며,
    클라이언트 코드에서 register_publisher() 메서드의 매개변수 타입은 CAgent*이다.
    따라서 다형성의 특성 때문에 클라이언트는 SpecialAgent의 구체적인 타입을 알지 못해도
    InitManager 객체가 test.xml 파일에 존재하는 Agent 요소들을 CSpecialAgent 타입으로 배열에 저장해도 별도의 코드 수정 없이 정상적으로 작동할 수 있다.

    다형성이란, 부모 클래스의 포인터로 자식 클래스의 객체를 참조할 수 있는 특성을 의미한다.
    따라서 클라이언트는 SpecialAgent의 내부 구현을 몰라도 부모 클래스인 CAgent의 인터페이스만으로 작업을 수행할 수 있다.
    */
    CManeuverManager mmanager;
    for (std::vector<CAgent *>::iterator iter = init_manager.get_agent_list().begin();
         iter != init_manager.get_agent_list().end(); ++iter)
    {
        mmanager.register_publisher(*iter);
    }

    /* Simulation Engine Initialization */
    double sim_time;
    double time_step = 1;

    for (sim_time = 0; sim_time < 30; sim_time += time_step)
    {
        // p_agent1->maneuver(time_step);
        // p_agent2->maneuver(time_step);

        // p_agent1->detect(p_agent2);
        // p_agent2->detect(p_agent1);

        mmanager.svc(time_step);

        // std::cout << "----" << std::endl;
        // std::cout << "Time: " << sim_time << ", " <<*p_agent1 << std::endl;
        // std::cout << "Time: " << sim_time << ", " <<*p_agent2 << std::endl;

        std::cout << "----" << std::endl;
        for (std::vector<CAgent *>::iterator iter = init_manager.get_agent_list().begin();
             iter != init_manager.get_agent_list().end(); ++iter)
        {
            std::cout << "Time: " << sim_time << ", " << *(*iter) << std::endl;
        }
    }
    return 0;
}
