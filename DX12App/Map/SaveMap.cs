using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;

public class SaveMap : MonoBehaviour
{
    void Start()
    {
        Debug.Log("Start Writing\r\n");
        FileStream file = new FileStream(Application.dataPath + "/MapData.tmap", FileMode.Create);
        StreamWriter writer = new StreamWriter(file);
        for (int i = 0; i < transform.childCount; ++i)
        {
            GameObject obj = transform.GetChild(i).gameObject;

            //메쉬 이름
            string name = obj.name;

            //좌표
            Vector3 pos = obj.transform.position;

            //쿼터니언 회전각
            var orientation = obj.transform.rotation;

            Vector3 scale = obj.transform.localScale;

            writer.Write(name + " " + pos.x.ToString() + " " + pos.y.ToString() + " " + pos.z.ToString() +
                " " + orientation.x.ToString() + " " + orientation.y.ToString() + " " + orientation.z.ToString() + " " + orientation.w.ToString() + " " + scale.x.ToString() + " " + scale.y.ToString() + " " + scale.z.ToString() + "\r\n");

            Debug.Log("Successfully Write " + name + "\r\n");
            Debug.Log(name + " Transform :" + "\r\n" + obj.transform.localToWorldMatrix.ToString() + "\r\n");
        }
        writer.Flush();
        writer.Close();
    }

    void Update()
    {
        
    }
}
