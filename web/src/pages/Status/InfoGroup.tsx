import { Card } from "components/Card";
import { type JSX } from "preact";
import { InfoItem, type InfoItemData } from "./InfoItem";

interface InfoGroupProps {
  name: string;
  items: InfoItemData[];
}

export { type InfoItemData };

export function InfoGroup(props: InfoGroupProps): JSX.Element {
  return (
    <>
      <Card title={props.name}>
        <ul className="list-group">
          {props.items.map((item) => {
            return (
              <InfoItem
                key={`${this.props.name}:${item.name}`}
                name={item.name}
                value={item.value}
                group={item.group}
                order={item.order}
              />
            );
          })}
        </ul>
      </Card>
    </>
  );
}
