import { memo } from "preact/compat";

export interface InfoItemData {
  name: string;
  value: string;
  group: string;
  order: number;
}

export const InfoItem = memo((props: InfoItemData) => {
  return (
    <div className="InfoItem">
      <li className="list-group-item d-flex justify-content-between align-items-start">
        <div className="fw-bold">{props.name}</div>
        {props.value}
      </li>
    </div>
  );
});
