import type { ContentItem } from '../../types/content';
import { Card } from '../ui/Card';

interface FeatureGridProps {
  items: ContentItem[];
  columns?: 2 | 3 | 4;
}

const columnClass: Record<number, string> = {
  2: 'lg:grid-cols-2',
  3: 'lg:grid-cols-3',
  4: 'lg:grid-cols-4',
};

export function FeatureGrid({ items, columns = 3 }: FeatureGridProps) {
  return (
    <div className={`grid grid-cols-1 md:grid-cols-2 ${columnClass[columns]} gap-6`}>
      {items.map((item) => (
        <Card
          key={item.title}
          title={item.title}
          description={item.description}
          icon={item.icon}
          href={item.href}
          cta={item.cta}
        />
      ))}
    </div>
  );
}
